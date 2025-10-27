#ifndef MENU_STATE_HPP
#define MENU_STATE_HPP

#include "GameState.hpp"
class Game;

class MenuState : public GameState {
public:
    void Enter(Game* game) override;
    void HandleInput(Game* game) override;
    void Update(Game* game, float dt) override;
    void Render(Game* game) override;
    void Exit(Game* game) override;
private:
    int current_option = 0;
};
#endif