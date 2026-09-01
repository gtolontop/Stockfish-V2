/*
  StockHuman, a human-like move selection layer for Stockfish.

  This layer does not replace Stockfish search. It uses Stockfish root
  candidates as a hidden judge, then chooses a plausible human move and
  optional response delay from those candidates.
*/

#ifndef STOCKHUMAN_H_INCLUDED
#define STOCKHUMAN_H_INCLUDED

#include <string>

#include "search.h"
#include "types.h"

namespace Stockfish {

class OptionsMap;
class Position;

namespace StockHuman {

struct Decision {
    Move        move = Move::none();
    std::string reason;
    int         candidateIndex = 0;
    int         evalLoss      = 0;
    int         lossBudget    = 0;
    int         naturalness   = 0;
    int         complexity    = 0;
    int         pressure      = 0;
    std::string flags;
};

struct TimePlan {
    TimePoint   target = TimePoint(0);
    std::string reason;
};

bool  enabled(const OptionsMap& options);
usize candidate_count(const OptionsMap& options, usize legalMoveCount);

Decision choose_move(const OptionsMap&        options,
                     const Search::LimitsType& limits,
                     const Position&          pos,
                     const Search::RootMoves& rootMoves,
                     usize                    multiPV);

TimePoint search_time_limit(const OptionsMap&         options,
                            const Search::LimitsType& limits,
                            const Position&           pos,
                            const Search::RootMoves&  rootMoves,
                            usize                     multiPV,
                            Depth                     rootDepth);

TimePlan plan_think_time(const OptionsMap&          options,
                         const Search::LimitsType&  limits,
                         const Position&            pos,
                         const Search::RootMoves&   rootMoves,
                         const StockHuman::Decision& decision);

}  // namespace StockHuman
}  // namespace Stockfish

#endif  // #ifndef STOCKHUMAN_H_INCLUDED
