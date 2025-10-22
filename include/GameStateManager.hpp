#ifndef GAME_STATE_MANAGER_HPP
#define GAME_STATE_MANAGER_HPP

#include <memory>
#include "GameState.hpp"

class Game;

class GameStateManager {
public:
    GameStateManager();

    void ChangeState(GameState* new_state, Game* game);

    void HandleInput(Game* game);
    void Update(Game* game, float dt);
    void Render(Game* game);

private:
    GameState* active_state; // raw pointer, does NOT own state
};

#endif