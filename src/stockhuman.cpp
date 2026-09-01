/*
  StockHuman, a human-like move selection layer for Stockfish.

  Stockfish remains the hidden tactical judge. StockHuman builds a small
  human-plausibility model on top of the searched root candidates: natural
  move priors, bounded mistakes, position complexity, clock pressure and a
  response-time plan. The boundary also leaves room for a later Maia/Lichess
  policy file without depending on external code.
*/

#include "stockhuman.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "bitboard.h"
#include "misc.h"
#include "movegen.h"
#include "position.h"
#include "ucioption.h"

namespace Stockfish::StockHuman {
namespace {

enum class Style {
    Balanced,
    Solid,
    Aggressive,
    Tactical,
    Positional
};

enum class Phase {
    Opening,
    Middlegame,
    Endgame
};

struct Settings {
    bool        active;
    int         elo;
    Style       style;
    int         consistency;
    int         tilt;
    int         thinkTime;
    int         openingKnowledge;
    int         adaptation;
    int         seed;
    std::string policyFile;
    int         policyMix;
};

struct MoveProfile {
    double      natural = 0.0;
    double      forcing = 0.0;
    double      risk    = 0.0;
    bool        capture = false;
    bool        check = false;
    bool        castling = false;
    bool        promotion = false;
    bool        safeCapture = false;
    bool        badCapture = false;
    bool        quiet = false;
    bool        obvious = false;
    std::string flags;
};

struct RootContext {
    Phase       phase = Phase::Middlegame;
    int         closeCount = 0;
    int         forcingCount = 0;
    int         topGap = 0;
    double      complexity = 0.0;
    double      pressure = 0.0;
    bool        inCheck = false;
    bool        obviousTactic = false;
    bool        hiddenEngineMove = false;
    MoveProfile topProfile;
};

struct ScoredMove {
    Move        move = Move::none();
    int         index = 0;
    int         loss = 0;
    double      score = -1e100;
    MoveProfile profile;
};

struct PolicyMove {
    std::string move;
    double      weight;
};

struct PolicyEntry {
    std::string             key;
    std::vector<PolicyMove> moves;
};

struct PolicyCache {
    std::string              path;
    std::vector<PolicyEntry> entries;
};

PolicyCache Policy;

double clamp01(double v) { return std::clamp(v, 0.0, 1.0); }

double unit(PRNG& rng) { return double(rng.rand<unsigned>() % 1000000) / 1000000.0; }

std::string lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });
    return str;
}

std::string trim(const std::string& str) {
    const auto first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";

    const auto last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void append_flag(std::string& flags, const char* flag) {
    if (!flags.empty())
        flags += ",";
    flags += flag;
}

Settings read_settings(const OptionsMap& options) {
    Style style = Style::Balanced;
    auto  styleName = lower(std::string(options["HumanStyle"]));

    if (styleName == "solid")
        style = Style::Solid;
    else if (styleName == "aggressive")
        style = Style::Aggressive;
    else if (styleName == "tactical")
        style = Style::Tactical;
    else if (styleName == "positional")
        style = Style::Positional;

    return {bool(options["StockHuman"]),
            int(options["HumanElo"]),
            style,
            int(options["HumanConsistency"]),
            int(options["HumanTilt"]),
            int(options["HumanThinkTime"]),
            int(options["HumanOpeningKnowledge"]),
            int(options["HumanAdaptation"]),
            int(options["HumanRandomSeed"]),
            std::string(options["HumanPolicyFile"]),
            int(options["HumanPolicyMix"])};
}

std::string describe_style(Style s) {
    switch (s)
    {
    case Style::Solid :
        return "solid";
    case Style::Aggressive :
        return "aggressive";
    case Style::Tactical :
        return "tactical";
    case Style::Positional :
        return "positional";
    case Style::Balanced :
        return "balanced";
    }

    return "balanced";
}

std::string phase_name(Phase p) {
    switch (p)
    {
    case Phase::Opening :
        return "opening";
    case Phase::Endgame :
        return "endgame";
    case Phase::Middlegame :
        return "middlegame";
    }

    return "middlegame";
}

char promotion_char(PieceType pt) {
    switch (pt)
    {
    case KNIGHT :
        return 'n';
    case BISHOP :
        return 'b';
    case ROOK :
        return 'r';
    case QUEEN :
        return 'q';
    default :
        return 'q';
    }
}

std::string square_to_string(Square s) {
    std::string out;
    out += char('a' + file_of(s));
    out += char('1' + rank_of(s));
    return out;
}

std::string move_to_uci(Move move) {
    if (!move)
        return "none";

    std::string out = square_to_string(move.from_sq()) + square_to_string(move.to_sq());
    if (move.type_of() == PROMOTION)
        out += promotion_char(move.promotion_type());
    return out;
}

Phase phase_of(const Position& pos) {
    const int pieces = popcount(pos.pieces());
    const int nonPawnMaterial =
      int(pos.non_pawn_material(WHITE) + pos.non_pawn_material(BLACK));

    if (pos.game_ply() < 20 && pieces >= 26
        && nonPawnMaterial > 2 * (QueenValue + 2 * RookValue))
        return Phase::Opening;

    if (pieces <= 12 || nonPawnMaterial <= 2 * (RookValue + BishopValue))
        return Phase::Endgame;

    return Phase::Middlegame;
}

double time_pressure(const Search::LimitsType& limits, const Position& pos) {
    const TimePoint remaining = limits.time[pos.side_to_move()];

    if (!remaining)
        return 0.0;

    const double inc = double(limits.inc[pos.side_to_move()]);
    const double softClock = double(remaining) + 0.7 * inc;

    if (softClock < 3500.0)
        return 1.0;
    if (softClock < 10000.0)
        return 0.78;
    if (softClock < 30000.0)
        return 0.48;
    if (softClock < 75000.0)
        return 0.18;

    return 0.0;
}

double fatigue(const Position& pos, const Settings& s) {
    if (pos.game_ply() < 55)
        return 0.0;

    const double longGame = clamp01((pos.game_ply() - 55) / 55.0);
    const double unstable = (100 - s.consistency + s.tilt) / 170.0;
    return clamp01(longGame * unstable);
}

double center_pull(Square sq) {
    const double f = std::abs(double(file_of(sq)) - 3.5);
    const double r = std::abs(double(rank_of(sq)) - 3.5);
    return 0.20 - 0.045 * (f + r);
}

MoveProfile profile_move(const Settings& s, const Position& pos, Move move, int loss) {
    MoveProfile p;

    const Color     us = pos.side_to_move();
    const Square    from = move.from_sq();
    const Square    to = move.to_sq();
    const Piece     piece = pos.moved_piece(move);
    const PieceType pt = type_of(piece);
    const PieceType captured = pos.capture(move) && pos.piece_on(to) != NO_PIECE
                               ? type_of(pos.piece_on(to))
                               : NO_PIECE_TYPE;
    const Phase phase = phase_of(pos);

    p.capture = pos.capture(move);
    p.check = pos.gives_check(move);
    p.castling = move.type_of() == CASTLING;
    p.promotion = move.type_of() == PROMOTION;
    p.quiet = !p.capture && !p.check && !p.promotion;

    if (p.capture)
    {
        p.safeCapture = pos.see_ge(move, 0);
        p.badCapture  = !pos.see_ge(move, -PawnValue / 2);
    }

    if (p.capture)
    {
        p.forcing += p.safeCapture ? 0.48 : 0.18;
        p.natural += p.safeCapture ? 0.26 : -0.18;
        append_flag(p.flags, p.safeCapture ? "safe-capture" : "capture");

        if (captured >= ROOK)
        {
            p.natural += 0.22;
            append_flag(p.flags, "material");
        }

        if (p.badCapture && loss > PawnValue / 2)
        {
            p.risk += 0.45;
            p.natural -= 0.38;
            append_flag(p.flags, "loose-capture");
        }
    }

    if (p.check)
    {
        p.forcing += 0.34;
        p.natural += loss < PawnValue ? 0.12 : -0.08;
        append_flag(p.flags, "check");
    }

    if (p.castling)
    {
        p.natural += phase == Phase::Opening ? 0.62 : 0.36;
        append_flag(p.flags, "castle");
    }

    if (p.promotion)
    {
        p.forcing += 0.70;
        p.natural += 0.70;
        append_flag(p.flags, "promotion");
    }

    if (pos.checkers())
    {
        p.forcing += 0.28;
        append_flag(p.flags, "evasion");
    }

    p.natural += center_pull(to);

    if (phase == Phase::Opening)
    {
        const Square rFrom = relative_square(us, from);
        const Square rTo   = relative_square(us, to);

        if (pt == PAWN)
        {
            if ((rFrom == SQ_E2 && (rTo == SQ_E4 || rTo == SQ_E3))
                || (rFrom == SQ_D2 && (rTo == SQ_D4 || rTo == SQ_D3)))
            {
                p.natural += 0.42;
                append_flag(p.flags, "center");
            }
            else if ((rFrom == SQ_C2 && rTo == SQ_C4) || (rFrom == SQ_G2 && rTo == SQ_G3))
            {
                p.natural += 0.18;
                append_flag(p.flags, "repertoire");
            }
            else if ((file_of(rFrom) == FILE_A || file_of(rFrom) == FILE_H) && !p.capture)
            {
                p.natural -= 0.32;
                p.risk += 0.18;
                append_flag(p.flags, "wing-pawn");
            }
        }
        else if (pt == KNIGHT)
        {
            if ((rFrom == SQ_G1 && (rTo == SQ_F3 || rTo == SQ_H3))
                || (rFrom == SQ_B1 && (rTo == SQ_C3 || rTo == SQ_D2)))
            {
                p.natural += rTo == SQ_F3 || rTo == SQ_C3 ? 0.42 : 0.10;
                append_flag(p.flags, "develop");
            }
            else if (!p.capture && !p.check)
            {
                p.natural -= 0.12;
                append_flag(p.flags, "second-move");
            }
        }
        else if (pt == BISHOP)
        {
            if (rFrom == SQ_F1 || rFrom == SQ_C1)
            {
                p.natural += 0.34;
                append_flag(p.flags, "develop");
            }
        }
        else if (pt == QUEEN && !p.capture && !p.check)
        {
            p.natural -= 0.55;
            p.risk += 0.25;
            append_flag(p.flags, "early-queen");
        }
        else if (pt == ROOK && !p.capture && !p.check && !p.castling)
        {
            p.natural -= 0.34;
            append_flag(p.flags, "early-rook");
        }
        else if (pt == KING && !p.castling)
        {
            p.natural -= 0.42;
            append_flag(p.flags, "king-walk");
        }
    }
    else if (phase == Phase::Endgame)
    {
        if (pt == KING)
        {
            p.natural += 0.30 + center_pull(to);
            append_flag(p.flags, "king-activity");
        }

        if (pt == PAWN && relative_rank(us, to) >= RANK_6)
        {
            p.natural += 0.26;
            append_flag(p.flags, "passed-pawn");
        }
    }

    if (p.quiet && pos.see_ge(move, -PawnValue / 3))
        p.natural += 0.08;
    else if (p.quiet)
    {
        p.natural -= 0.24;
        p.risk += 0.28;
        append_flag(p.flags, "hanging-risk");
    }

    switch (s.style)
    {
    case Style::Solid :
        p.natural += p.castling ? 0.18 : 0.0;
        p.natural += p.quiet && loss < PawnValue / 2 ? 0.08 : 0.0;
        p.natural -= p.risk * 0.45;
        break;
    case Style::Aggressive :
        p.natural += p.check ? 0.20 : 0.0;
        p.natural += p.safeCapture ? 0.12 : 0.0;
        break;
    case Style::Tactical :
        p.natural += p.check ? 0.25 : 0.0;
        p.natural += p.safeCapture ? 0.16 : 0.0;
        p.forcing += p.promotion ? 0.20 : 0.0;
        break;
    case Style::Positional :
        p.natural += p.quiet ? 0.16 : 0.0;
        p.natural -= loss > PawnValue ? 0.24 : 0.0;
        break;
    case Style::Balanced :
        break;
    }

    p.obvious = (p.safeCapture && loss < PawnValue / 3)
             || (p.castling && phase == Phase::Opening && loss < PawnValue / 2)
             || (p.promotion && loss < PawnValue / 2);

    return p;
}

int close_alternatives(const Search::RootMoves& rootMoves, usize multiPV, int window) {
    if (rootMoves.empty())
        return 0;

    const Value top = rootMoves[0].score;
    int         n   = 0;

    for (usize i = 0; i < multiPV; ++i)
        if (top - rootMoves[i].score <= window)
            ++n;

    return n;
}

int forcing_candidates(const Settings&           s,
                       const Position&           pos,
                       const Search::RootMoves&  rootMoves,
                       usize                     multiPV) {
    int count = 0;

    for (usize i = 0; i < multiPV; ++i)
    {
        if (rootMoves[i].pv.empty())
            continue;

        const int loss = std::max(0, int(rootMoves[0].score - rootMoves[i].score));
        auto      p    = profile_move(s, pos, rootMoves[i].pv[0], loss);

        if (p.capture || p.check || p.promotion)
            ++count;
    }

    return count;
}

RootContext root_context(const Settings&          s,
                         const Search::LimitsType& limits,
                         const Position&          pos,
                         const Search::RootMoves& rootMoves,
                         usize                    multiPV) {
    RootContext ctx;

    ctx.phase = phase_of(pos);
    ctx.inCheck = bool(pos.checkers());
    ctx.pressure = time_pressure(limits, pos);

    if (rootMoves.empty())
        return ctx;

    multiPV = std::min(multiPV, rootMoves.size());
    ctx.closeCount = close_alternatives(rootMoves, multiPV, PawnValue / 2);
    ctx.forcingCount = forcing_candidates(s, pos, rootMoves, multiPV);

    if (rootMoves[0].pv.size())
        ctx.topProfile = profile_move(s, pos, rootMoves[0].pv[0], 0);

    if (multiPV > 1)
        ctx.topGap = std::max(0, int(rootMoves[0].score - rootMoves[1].score));

    ctx.obviousTactic = ctx.topGap > 3 * PawnValue / 4
                     && (ctx.topProfile.safeCapture || ctx.topProfile.check
                         || ctx.topProfile.promotion);

    ctx.hiddenEngineMove = ctx.topGap > PawnValue && ctx.topProfile.quiet
                        && ctx.topProfile.natural < 0.10 && !ctx.inCheck;

    ctx.complexity = 0.12 * std::max(0, ctx.closeCount - 1)
                   + 0.08 * std::min(8, ctx.forcingCount)
                   + (ctx.inCheck ? 0.22 : 0.0)
                   + (ctx.topGap < PawnValue / 3 && multiPV > 1 ? 0.20 : 0.0)
                   + (ctx.obviousTactic ? 0.18 : 0.0)
                   + (ctx.hiddenEngineMove ? 0.28 : 0.0);

    if (ctx.phase == Phase::Endgame && ctx.forcingCount <= 2)
        ctx.complexity += 0.24;

    ctx.complexity = clamp01(ctx.complexity);
    return ctx;
}

int elo_loss_budget(const Settings& s, const RootContext& ctx, Value topScore) {
    const int e = std::clamp(s.elo, 800, 3000);

    int budget = int(interpolate(e, 800, 3000, 2.55 * PawnValue, 0.20 * PawnValue));

    budget += (100 - s.consistency) * int(PawnValue) / 95;
    budget += s.tilt * int(PawnValue) / 165;
    budget += int(ctx.pressure * interpolate(e, 800, 3000, 0.85 * PawnValue, 0.22 * PawnValue));
    budget += int(ctx.complexity * interpolate(e, 800, 3000, 0.55 * PawnValue, 0.12 * PawnValue));

    if (ctx.obviousTactic)
        budget -= int(0.28 * PawnValue);
    if (ctx.hiddenEngineMove)
        budget += int(0.30 * PawnValue);

    if (s.style == Style::Solid || s.style == Style::Positional)
        budget -= int(0.14 * PawnValue);
    else if (s.style == Style::Aggressive || s.style == Style::Tactical)
        budget += int(0.10 * PawnValue);

    if (s.adaptation > 0)
    {
        if (topScore > 2 * PawnValue)
            budget += int(topScore) * s.adaptation / 360;
        else if (topScore < -2 * PawnValue)
            budget -= int(-topScore) * s.adaptation / 430;
    }

    return std::clamp(budget, int(0.20 * PawnValue), int(3.2 * PawnValue));
}

bool allow_blunder_window(const Settings& s, const RootContext& ctx, PRNG& rng) {
    const int    e = std::clamp(s.elo, 800, 2800);
    const double weakness = double(2800 - e) / 2000.0;
    double       chance = 0.004 + 0.060 * weakness * weakness;

    chance *= 1.0 + s.tilt / 85.0 + (100 - s.consistency) / 150.0;
    chance *= 1.0 + ctx.pressure * 1.4 + ctx.complexity * 0.45;

    if (ctx.obviousTactic && e >= 1700)
        chance *= 0.45;

    return unit(rng) < chance;
}

int hard_loss_limit(const Settings& s, const RootContext& ctx, int budget, bool blunderWindow) {
    const int e = std::clamp(s.elo, 800, 2800);
    int       hard = budget + int(interpolate(e, 800, 2800, 2.20 * PawnValue, 0.45 * PawnValue));

    hard += s.tilt * int(PawnValue) / 90;
    hard += int(ctx.pressure * 0.55 * PawnValue);

    if (!blunderWindow)
        hard = std::min(hard, budget + int(0.80 * PawnValue));
    else if (e >= 1800)
        hard = std::min(hard, int(3.0 * PawnValue));

    return std::clamp(hard, budget, int(5.2 * PawnValue));
}

std::string position_key(const Position& pos) {
    std::istringstream fen(pos.fen());
    std::string        board, stm, castling, ep;

    fen >> board >> stm >> castling >> ep;
    return board + " " + stm + " " + castling + " " + ep;
}

void load_policy_file(const std::string& path) {
    if (Policy.path == path)
        return;

    Policy.path = path;
    Policy.entries.clear();

    if (path.empty())
        return;

    std::ifstream in(path);
    if (!in)
        return;

    std::string line;
    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        const auto sep = line.find('|');
        if (sep == std::string::npos)
            continue;

        PolicyEntry entry;
        entry.key = trim(line.substr(0, sep));

        std::istringstream moves(line.substr(sep + 1));
        std::string        move;
        double             weight;

        while (moves >> move >> weight)
            if (weight > 0.0)
                entry.moves.push_back({lower(move), weight});

        if (!entry.key.empty() && !entry.moves.empty())
            Policy.entries.push_back(entry);
    }
}

double external_policy_prior(const Settings& s, const Position& pos, Move move) {
    if (s.policyMix <= 0 || s.policyFile.empty())
        return 0.0;

    load_policy_file(s.policyFile);

    const std::string key = position_key(pos);
    const std::string uci = lower(move_to_uci(move));

    for (const auto& entry : Policy.entries)
    {
        if (entry.key != key)
            continue;

        double best = 0.0;
        double found = 0.0;

        for (const auto& pm : entry.moves)
        {
            best = std::max(best, pm.weight);
            if (pm.move == uci)
                found = pm.weight;
        }

        const double mix = double(s.policyMix) / 100.0;
        if (found > 0.0 && best > 0.0)
            return mix * (0.75 + std::log(std::max(0.02, found / best)));

        return -0.35 * mix;
    }

    return 0.0;
}

bool candidate_is_absurd(const Settings&    s,
                         const RootContext& ctx,
                         const MoveProfile& profile,
                         int                loss,
                         int                hardLimit,
                         bool               blunderWindow,
                         usize              index) {
    if (index == 0)
        return false;

    if (loss > hardLimit)
        return true;

    const int e = std::clamp(s.elo, 800, 2800);

    if (!blunderWindow && loss > hardLimit - PawnValue / 4)
        return true;

    if (e >= 1800 && loss > int(2.65 * PawnValue) && !ctx.hiddenEngineMove)
        return true;

    if (profile.badCapture && loss > PawnValue)
        return true;

    if (profile.risk > 0.50 && loss > 3 * PawnValue / 2 && e >= 1600)
        return true;

    if (ctx.obviousTactic && loss > PawnValue && e >= 1700)
        return true;

    return false;
}

int minimum_depth(const Settings& s, const RootContext& ctx) {
    int depth = int(interpolate(std::clamp(s.elo, 800, 2800), 800, 2800, 4.0, 10.0));

    if (ctx.obviousTactic || ctx.inCheck)
        depth += 1;
    if (ctx.phase == Phase::Endgame)
        depth += 1;
    if (ctx.pressure > 0.7)
        depth -= 2;
    else if (ctx.pressure > 0.3)
        depth -= 1;

    return std::clamp(depth, 3, 11);
}

double response_time_ms(const Settings&          s,
                        const Search::LimitsType& limits,
                        const Position&          pos,
                        const Search::RootMoves& rootMoves,
                        usize                    multiPV,
                        Move                     move,
                        int                      loss,
                        bool                     finalMove) {
    if (!s.active || s.thinkTime <= 0 || rootMoves.empty() || limits.infinite
        || limits.ponderMode || limits.depth || limits.nodes || limits.mate || limits.movetime)
        return 0.0;

    multiPV = std::min(multiPV, rootMoves.size());
    const RootContext ctx = root_context(s, limits, pos, rootMoves, multiPV);
    const MoveProfile profile =
      move ? profile_move(s, pos, move, loss) : ctx.topProfile;

    double ms = 520.0;

    if (rootMoves.size() == 1)
        ms = 150.0;
    else
    {
        ms += 190.0 * std::max(0, ctx.closeCount - 1);
        ms += 1350.0 * ctx.complexity;
        ms += std::min(1300.0, double(loss) * 2.6);

        if (ctx.inCheck)
            ms += rootMoves.size() <= 3 ? 120.0 : 520.0;

        if (profile.obvious && !ctx.hiddenEngineMove)
            ms -= 470.0;
        else if (profile.safeCapture && loss < PawnValue / 2)
            ms -= 260.0;

        if (ctx.obviousTactic && finalMove)
            ms += 420.0;

        if (ctx.phase == Phase::Opening)
            ms *= interpolate(s.openingKnowledge, 0, 100, 1.28, 0.42);
        else if (ctx.phase == Phase::Endgame && profile.forcing < 0.40)
            ms += 780.0;
    }

    ms *= interpolate(std::clamp(s.elo, 800, 2800), 800, 2800, 0.72, 1.18);
    ms *= interpolate(s.consistency, 0, 100, 1.18, 0.94);
    ms *= 1.0 + fatigue(pos, s) * 0.35;
    ms *= double(s.thinkTime) / 100.0;

    const TimePoint remaining = limits.time[pos.side_to_move()];
    if (remaining)
    {
        const double clock = double(remaining);
        const double inc = double(limits.inc[pos.side_to_move()]);

        if (clock < 3500.0)
            ms = std::min(ms, 260.0 + inc * 0.18);
        else if (clock < 10000.0)
            ms = std::min(ms, 650.0 + inc * 0.28);
        else if (clock < 30000.0)
            ms = std::min(ms, 1650.0 + inc * 0.42);
        else
            ms = std::min(ms, std::max(650.0, clock * 0.035 + inc * 0.60));
    }

    if (profile.obvious && loss < PawnValue / 3)
        ms = std::min(ms, 850.0 * double(s.thinkTime) / 100.0 + 80.0);

    return std::clamp(ms, 70.0, 45000.0);
}

}  // namespace

bool enabled(const OptionsMap& options) { return bool(options["StockHuman"]); }

usize candidate_count(const OptionsMap& options, usize legalMoveCount) {
    const Settings s = read_settings(options);
    if (!s.active)
        return 0;

    int count = 7 + (2800 - std::clamp(s.elo, 800, 2800)) / 150;
    count += (100 - s.consistency) / 20;
    count += s.tilt / 28;

    if (s.style == Style::Tactical || s.style == Style::Aggressive)
        count += 2;
    if (s.policyMix > 0)
        count += 2;

    return std::min(legalMoveCount, usize(std::clamp(count, 5, 24)));
}

Decision choose_move(const OptionsMap&        options,
                     const Search::LimitsType& limits,
                     const Position&          pos,
                     const Search::RootMoves& rootMoves,
                     usize                    multiPV) {
    Decision decision;

    if (rootMoves.empty())
        return decision;

    const Settings s = read_settings(options);
    multiPV          = std::min(multiPV, rootMoves.size());

    if (!s.active || multiPV == 1)
    {
        decision.move   = rootMoves[0].pv[0];
        decision.reason = "StockHuman disabled-or-single-candidate";
        return decision;
    }

    const RootContext ctx           = root_context(s, limits, pos, rootMoves, multiPV);
    const Value       topScore      = rootMoves[0].score;
    const int         budget        = elo_loss_budget(s, ctx, topScore);
    const u64         baseSeed =
      s.seed ? u64(s.seed) : (u64(now()) ^ u64(pos.key()) ^ (u64(pos.game_ply()) << 32));
    PRNG rng(baseSeed ? baseSeed : 1);

    const bool blunderWindow = allow_blunder_window(s, ctx, rng);
    const int  hardLimit     = hard_loss_limit(s, ctx, budget, blunderWindow);
    const int  lossLimit     = blunderWindow ? hardLimit : budget;

    std::vector<ScoredMove> scored;
    scored.reserve(multiPV);

    for (usize i = 0; i < multiPV; ++i)
    {
        if (rootMoves[i].pv.empty())
            continue;

        const int loss = std::max(0, int(topScore - rootMoves[i].score));
        const Move move = rootMoves[i].pv[0];
        MoveProfile profile = profile_move(s, pos, move, loss);

        if (loss > lossLimit && i != 0)
            continue;
        if (candidate_is_absurd(s, ctx, profile, loss, hardLimit, blunderWindow, i))
            continue;

        const double lossRatio = double(loss) / double(std::max(1, budget));
        const double qualityPenalty =
          std::pow(lossRatio, 1.18) * (1.25 + s.consistency / 135.0);
        const double rankPenalty = std::log(double(i + 1)) * 0.22;
        const double riskPenalty = profile.risk * (0.80 + s.consistency / 160.0);
        const double policy      = external_policy_prior(s, pos, move);
        const double hiddenBonus = ctx.hiddenEngineMove && i != 0 && profile.natural > ctx.topProfile.natural
                                   ? 0.20
                                   : 0.0;

        double humanScore = profile.natural + 0.36 * profile.forcing + policy + hiddenBonus
                          - qualityPenalty - rankPenalty - riskPenalty;

        if (ctx.obviousTactic && i == 0)
            humanScore += 0.38;
        if (ctx.obviousTactic && i != 0)
            humanScore -= 0.30;
        if (ctx.inCheck && profile.quiet && !profile.safeCapture)
            humanScore -= 0.18;

        scored.push_back({move, int(i), loss, humanScore, profile});
    }

    if (scored.empty())
        scored.push_back({rootMoves[0].pv[0], 0, 0, 0.0, profile_move(s, pos, rootMoves[0].pv[0], 0)});

    double bestScore = scored[0].score;
    for (const auto& item : scored)
        bestScore = std::max(bestScore, item.score);

    const double temperature =
      0.18 + (100 - s.consistency) / 260.0 + s.tilt / 340.0 + ctx.pressure * 0.14
      + fatigue(pos, s) * 0.08
      + interpolate(std::clamp(s.elo, 800, 2800), 800, 2800, 0.18, 0.035);

    double total = 0.0;
    for (auto& item : scored)
    {
        item.score = std::exp((item.score - bestScore) / std::max(0.05, temperature));
        total += item.score;
    }

    double pick = unit(rng) * total;
    const ScoredMove* selected = &scored.front();
    for (const auto& item : scored)
    {
        pick -= item.score;
        if (pick <= 0.0)
        {
            selected = &item;
            break;
        }
    }

    decision.move           = selected->move;
    decision.candidateIndex = selected->index;
    decision.evalLoss       = selected->loss;
    decision.lossBudget     = budget;
    decision.naturalness    = int(std::round(selected->profile.natural * 100.0));
    decision.complexity     = int(std::round(ctx.complexity * 100.0));
    decision.pressure       = int(std::round(ctx.pressure * 100.0));
    decision.flags          = selected->profile.flags.empty() ? "plain" : selected->profile.flags;

    std::ostringstream reason;
    reason << "StockHuman decision"
           << " elo=" << s.elo
           << " style=" << describe_style(s.style)
           << " move=" << move_to_uci(decision.move)
           << " idx=" << (decision.candidateIndex + 1)
           << " loss=" << decision.evalLoss
           << " budget=" << decision.lossBudget
           << " hard=" << hardLimit
           << " phase=" << phase_name(ctx.phase)
           << " complexity=" << decision.complexity
           << " pressure=" << decision.pressure
           << " natural=" << decision.naturalness
           << " flags=" << decision.flags;

    if (ctx.obviousTactic)
        reason << " tactic=obvious";
    if (ctx.hiddenEngineMove)
        reason << " engine_line=hidden";
    if (blunderWindow)
        reason << " mood=loose";

    decision.reason = reason.str();
    return decision;
}

TimePoint search_time_limit(const OptionsMap&         options,
                            const Search::LimitsType& limits,
                            const Position&           pos,
                            const Search::RootMoves&  rootMoves,
                            usize                     multiPV,
                            Depth                     rootDepth) {
    const Settings s = read_settings(options);

    if (!s.active || s.thinkTime <= 0 || rootMoves.empty() || limits.infinite
        || limits.ponderMode || limits.depth || limits.nodes || limits.mate || limits.movetime
        || !limits.use_time_management())
        return TimePoint(0);

    multiPV = std::min(multiPV, rootMoves.size());
    const RootContext ctx = root_context(s, limits, pos, rootMoves, multiPV);

    if (rootDepth < minimum_depth(s, ctx))
        return TimePoint(0);

    const double target = response_time_ms(s, limits, pos, rootMoves, multiPV,
                                           rootMoves[0].pv.empty() ? Move::none()
                                                                   : rootMoves[0].pv[0],
                                           0, false);
    const double searchShare =
      ctx.obviousTactic || ctx.hiddenEngineMove ? 0.92 : (ctx.topProfile.obvious ? 0.58 : 0.78);

    return TimePoint(std::clamp(int(target * searchShare), 90, 30000));
}

TimePlan plan_think_time(const OptionsMap&          options,
                         const Search::LimitsType&  limits,
                         const Position&            pos,
                         const Search::RootMoves&   rootMoves,
                         const StockHuman::Decision& decision) {
    TimePlan plan;
    const Settings s = read_settings(options);

    if (!s.active || decision.move == Move::none() || rootMoves.empty())
        return plan;

    const usize multiPV = std::min(rootMoves.size(), candidate_count(options, rootMoves.size()));
    const double target =
      response_time_ms(s, limits, pos, rootMoves, multiPV, decision.move, decision.evalLoss, true);

    plan.target = TimePoint(int(target));

    if (plan.target)
    {
        std::ostringstream reason;
        reason << "StockHuman time"
               << " target=" << plan.target
               << " move=" << move_to_uci(decision.move)
               << " idx=" << (decision.candidateIndex + 1)
               << " loss=" << decision.evalLoss
               << " complexity=" << decision.complexity
               << " pressure=" << decision.pressure
               << " flags=" << (decision.flags.empty() ? "plain" : decision.flags);
        plan.reason = reason.str();
    }

    return plan;
}

}  // namespace Stockfish::StockHuman
