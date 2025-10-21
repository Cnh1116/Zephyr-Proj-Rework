#ifndef PLAY_STATE_HPP
#define PLAY_STATE_HPP

#include "GameState.hpp"
class Game;

class PlayState : public GameState {
public:
    void Enter(Game* game) override;
    void HandleInput(Game* game, SDL_Event& e) override;
    void Update(Game* game, float dt) override;
    void Render(Game* game) override;
    void Exit(Game* game) override;
};
#endif