#ifndef PAUSE_STATE_HPP
#define PAUSE_STATE_HPP

#include "GameState.hpp"
class Game;

class PauseState : public GameState {
public:
    void Enter(Game* game) override;
    void HandleInput(Game* game) override;
    void Update(Game* game, float dt) override;
    void Render(Game* game) override;
    void Exit(Game* game) override;
};
#endif