#pragma once

#include "Player.hpp"
#include "PlayerStream.hpp"

#include <iterator>
#include <unordered_map>
#include <vector>

struct RankingResult {
    /**
     * @brief The collection of top-ranked players.
     *
     * Contains the highest level players sorted in ascending order by level
     * (lowest level first, highest level last).
     */
    std::vector<Player> top_;

    /**
     * @brief Map of player count milestones to their respective minimum level cutoffs.
     *
     * Keys represent the number of players processed at a given point,
     * and values represent the minimum level required to be in the leaderboard
     * at that point.
     *
     * @example Assuming we have a stream with 132 players, consider:
     *
     *   { 50: 239, 100: 992, 132: 994 }
     *
     *   1) After fetching/processing 50 players,
     *   the minimum level to be on the leaderboard is 239.
     *   2) After fetching/processing 100 players,
     *   the minimum level to be on the leaderboard is 992.
     *   3) After fetching/processing ALL 132 players,
     *   the minimum level to be on the leaderboard is 994.
     */
    std::unordered_map<size_t, size_t> cutoffs_;

    /**
     * @brief Represents the total elapsed processing time for the entire ranking operation, in ms.
     */
    double elapsed_;

    /**
     * @brief Constructor for RankingResult with top players, cutoffs, and elapsed time.
     *
     * @param top Vector of top-ranked Player objects, in sorted order.
     * @param cutoffs Map of player count thresholds to minimum level cutoffs.
     *   NOTE: This is only ever non-empty for Online::rankIncoming().
     *         This parameter & the corresponding member should be empty
     *         for all Offline algorithms.
     * @param elapsed Time taken to calculate the ranking, in ms.
     */
    RankingResult(const std::vector<Player>& top = {}, const std::unordered_map<size_t, size_t>& cutoffs = {}, double elapsed = 0);
};

namespace Offline {
/**
 * @brief Uses a mixture of quickselect/quicksort to
 *        select and sort the top 10% of players with O(log N) memory
 *        (excluding the returned RankingResult vector)
 *
 * @param players A reference to the vector of Player objects to be ranked
 * @return A Ranking Result object whose
 * - top_ vector -> Contains the top 10% of players from the input in sorted order (ascending)
 * - cutoffs_    -> Is empty
 * - elapsed_    -> Contains the duration (ms) of the selection/sorting operation
 *
 * @post The order of the parameter vector is modified.
 */
RankingResult quickSelectRank(std::vector<Player>& players);

/**
 * @brief Uses an early-stopping version of heapsort to
 *        select and sort the top 10% of players in-place
 *        (excluding the returned RankingResult vector)
 *
 * @param players A reference to the vector of Player objects to be ranked
 * @return A Ranking Result object whose
 * - top_ vector -> Contains the top 10% of players from the input in sorted order (ascending)
 * - cutoffs_    -> Is empty
 * - elapsed_    -> Contains the duration (ms) of the selection/sorting operation
 *
 * @post The order of the parameter vector is modified.
 */
RankingResult heapRank(std::vector<Player>& players);
};

namespace Online {
using PlayerIt = std::vector<Player>::iterator;

/**
 * @brief A helper method that replaces the minimum element
 * in a min-heap with a target value & preserves the heap
 * by percolating the new value down to its correct position.
 *
 * Performs in O(log N) time.
 *
 * @pre The range [first, last) is a min-heap.
 *
 * @param first An iterator to a vector of Player objects
 *      denoting the beginning of a min-heap
 *      NOTE: Unlike the textbook, this is *not* an empty slot
 *      used to store temporary values. It is the root of the heap.
 *
 * @param last An iterator to a vector of Player objects
 *      denoting one past the end of a min-heap
 *      (i.e. it is not considering a valid index of the heap)
 *
 * @param target A reference to a Player object to be inserted into the heap
 * @post
 * - The vector slice denoted from [first,last) is a min-heap
 *   into which `target` has been inserted.
 * - The contents of `target` is not guaranteed to match its original state
 *   (ie. you may move it).
 */
void replaceMin(PlayerIt first, PlayerIt last, Player& target);

/**
 * @brief Exhausts a stream of Players (ie. until there are none left) such that we:
 * 1) Maintain a running collection of the <reporting_interval> highest leveled players
 * 2) Record the Player level after reading every <reporting_interval> players
 *    representing the minimum level required to be in the leaderboard at that point.
 *
 * @note You should use NOT use a priority-queue.
 *       Instead, use a vector, the STL heap operations, & `replaceMin()`
 *
 * @param stream A stream providing Player objects
 * @param reporting_interval The frequency at which to record cutoff levels
 * @return A RankingResult in which:
 * - top_       -> Contains the top <reporting_interval> Players read in the stream in
 *                 sorted (least to greatest) order
 * - cutoffs_   -> Maps player count milestones to minimum level required at that point
 *                 including the minimum level after ALL players have been read, regardless
 *                 of being a multiple of the reporting interval
 * - elapsed_   -> Contains the duration (ms) of the selection/sorting operation
 *                 excluding fetching the next player in the stream
 *
 * @post All elements of the stream are read until there are none remaining.
 *
 * @example Suppose we have:
 * 1) A stream with 132 players
 * 2) A reporting interval of 50
 *
 * Then our resulting RankingResult might contain something like:
 * top_ = { Player("RECLUSE", 994), Player("WYLDER", 1002), ..., Player("DUCHESS", 1399) }, with length 50
 * cutoffs_ = { 50: 239, 100: 992, 132: 994 } (see RankingResult explanation)
 * elapsed_ = 0.003 (Your runtime will vary based on hardware)
 */
RankingResult rankIncoming(PlayerStream& stream, const size_t& reporting_interval);
};