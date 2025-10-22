#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP
#include <SDL.h>

class Game;

class GameState {
public:
    virtual ~GameState() = default;
    virtual void Enter(Game* game) = 0;
    virtual void HandleInput(Game* game) = 0;
    virtual void Update(Game* game, float dt) = 0;
    virtual void Render(Game* game) = 0;
    virtual void Exit(Game* game) = 0;
};
#endif