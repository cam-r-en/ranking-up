#pragma once
#include <string>

struct Player {
    std::string name_;
    size_t level_;
    size_t id_;
    
    /**
    * @brief Constructs a Player with the given identifier.
    * @param name A const. string reference to be the player name
    * @param level The current level of the Player
    */
    Player(const std::string& name="NONE", const size_t& level = 1);

    /**
     * @brief Defines convenience comparators for Players, 
     * defining notions of equality & ordering on their level.
     */
    bool operator<(const Player& rhs) const;
    bool operator==(const Player& rhs) const;
    bool operator>(const Player& rhs) const;
};
