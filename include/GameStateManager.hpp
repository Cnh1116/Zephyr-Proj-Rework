#ifndef GAME_STATE_MANAGER_HPP
#define GAME_STATE_MANAGER_HPP

#include <memory>
#include "GameState.hpp"

class Game;

class GameStateManager {
private:
    std::unique_ptr<GameState> current_state;

public:
    void ChangeState(std::unique_ptr<GameState> new_state, Game* game);
    void HandleInput(Game* game, SDL_Event& e);
    void Update(Game* game, float dt);
    void Render(Game* game);
};
#endif