#include "Leaderboard.hpp"
#include <algorithm>
#include <chrono>

/**
 * @brief Constructor for RankingResult with top players, cutoffs, and elapsed time.
 *
 * @param top Vector of top-ranked Player objects, in sorted order.
 * @param cutoffs Map of player count thresholds to minimum level cutoffs.
 *   NOTE: This is only ever non-empty for Online::rankIncoming().
 *         This parameter & the corresponding member should be empty
 *         for all Offline algorithms.
 * @param elapsed Time taken to calculate the ranking, in seconds.
 */
RankingResult::RankingResult(const std::vector<Player>& top, const std::unordered_map<size_t, size_t>& cutoffs, double elapsed)
    : top_ { top }
    , cutoffs_ { cutoffs }
    , elapsed_ { elapsed }
{
}

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
RankingResult Offline::quickSelectRank(std::vector<Player>& players) {
    auto start = std::chrono::high_resolution_clock::now();

    size_t totalPlayers = players.size();
    size_t topCount = (totalPlayers + 9) / 10; // Ceiling of 10%

    // Quickselect to find the topCount-th largest element
    std::nth_element(players.begin(), players.end() - topCount, players.end());

    // Extract the top 10% players
    std::vector<Player> topPlayers(players.end() - topCount, players.end());

    // Sort the top players in ascending order
    std::sort(topPlayers.begin(), topPlayers.end());

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    return RankingResult(topPlayers, {}, elapsed);
}

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
RankingResult Offline::heapRank(std::vector<Player>& players) {
    auto start = std::chrono::high_resolution_clock::now();

    size_t totalPlayers = players.size();
    size_t topCount = (totalPlayers + 9) / 10; // Ceiling of 10%

    // Build a max heap
    std::make_heap(players.begin(), players.end());

    // Extract the topCount largest elements
    std::vector<Player> topPlayers;
    for (size_t i = 0; i < topCount; ++i) {
        std::pop_heap(players.begin(), players.end() - i);
        topPlayers.push_back(players[players.size() - 1 - i]);
    }

    // Sort the top players in ascending order
    std::sort(topPlayers.begin(), topPlayers.end());

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    return RankingResult(topPlayers, {}, elapsed);
}

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
void Online::replaceMin(PlayerIt first, PlayerIt last, Player& target) {
    size_t heapSize = std::distance(first, last);
    if (heapSize == 0) {
        return; // Empty heap, nothing to replace
    }

    // Replace the root of the heap with the target
    *first = std::move(target);

    // Percolate down to restore the min-heap property
    size_t index = 0;
    while (true) {
        size_t leftChildIdx = 2 * index + 1;
        size_t rightChildIdx = 2 * index + 2;
        size_t smallestIdx = index;

        if (leftChildIdx < heapSize && *(first + leftChildIdx) < *(first + smallestIdx)) {
            smallestIdx = leftChildIdx;
        }
        if (rightChildIdx < heapSize && *(first + rightChildIdx) < *(first + smallestIdx)) {
            smallestIdx = rightChildIdx;
        }
        if (smallestIdx == index) {
            break; // Heap property is restored
        }

        std::swap(*(first + index), *(first + smallestIdx));
        index = smallestIdx;
    }
}

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
RankingResult Online::rankIncoming(PlayerStream& stream, const size_t& reporting_interval) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<Player> topPlayers;
    std::unordered_map<size_t, size_t> cutoffs;

    size_t playerCount = 0;

    // Initialize the min-heap with the first 'reporting_interval' players
    while (playerCount < reporting_interval && stream.remaining() > 0) {
        topPlayers.push_back(stream.nextPlayer());
        playerCount++;
    }
    std::make_heap(topPlayers.begin(), topPlayers.end(), std::greater<Player>());

    // Record the cutoff after the initial batch if applicable
    if (playerCount == reporting_interval) {
        cutoffs[playerCount] = topPlayers.front().level_;
    }

    // Process remaining players in the stream
    while (stream.remaining() > 0) {
        Player next = stream.nextPlayer();
        playerCount++;

        // If the new player has a higher level than the minimum in the heap
        if (next.level_ > topPlayers.front().level_) {
            Online::replaceMin(topPlayers.begin(), topPlayers.end(), next);
        }

        // Record cutoff at each reporting interval
        if (playerCount % reporting_interval == 0) {
            cutoffs[playerCount] = topPlayers.front().level_;
        }
    }

    // Record final cutoff if not already recorded
    if (playerCount % reporting_interval != 0) {
        cutoffs[playerCount] = topPlayers.front().level_;
    }

    // Sort the top players in ascending order
    std::sort(topPlayers.begin(), topPlayers.end());

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    return RankingResult(topPlayers, cutoffs, elapsed);
}