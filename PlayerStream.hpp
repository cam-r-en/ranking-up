#pragma once
#include "Player.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Interface for fetching Player objects sequentially.
 */
class PlayerStream {
public:
    /**
     * @brief Retrieves the next Player in the stream, if possible.
     *
     * @return The next Player object in the sequence.
     * @post Updates stream members so a subsequent call to
     *      nextPlayer() will yield the next Player in the to-be-read
     *      sequence.
     *
     * @throws std::runtime_error, if there are no more players to fetch
     *      & nextPlayer() is called.
     */

    virtual Player nextPlayer() = 0;

    /**
     * @brief Returns the number of players remaining in the stream.

     * @return The count of players left to be read.
     */
    virtual size_t remaining() const = 0;
};

/**
 * @brief The interface for a PlayerStream created using the contents of a vector.
 *
 * @example Let's cover a brief example:
 * Given a vector of Player objects v = {
 *      Player("Rykard", 23),
 *      Player("Malenia", 99)
 *  }
 *
 * Our stream's behavior would be someting akin to:
 *
 * stream.remaining() -> 2
 * stream.nextPlayer() -> Player("Rykard", 23)
 * stream.nextPlayer() -> Player("Malenia", 99)
 * stream.remaining() -> 0
 * stream.nextPlayer() -> throws std::runtime_error()
 */
class VectorPlayerStream : public PlayerStream {
private:
    // Your private members here. You're the designer now!
    std::vector<Player> players_;
    size_t currentIndex_;

public:
    /**
     * @brief Constructs a VectorPlayerStream from a vector of Players.
     *
     * Initializes the stream with a sequence of Player objects matching the
     * contents of the given vector.
     *
     * @param players The vector of Player objects to stream.
     */
    VectorPlayerStream(const std::vector<Player>& players);

    /**
    * @brief Retrieves the next Player in the stream.
    *
    * @return The next Player object in the sequence.
    * @post Updates members so a subsequent call to nextPlayer() yields the Player
    * following that which is returned.

    * @throws std::runtime_error If there are no more players remaining in the stream.
    */
    Player nextPlayer() override;

    /**
     * @brief Returns the number of players remaining in the stream.
     *
     * @return The count of players left to be read.
     */
    size_t remaining() const override; // see how many instances remaining to be fetched
};