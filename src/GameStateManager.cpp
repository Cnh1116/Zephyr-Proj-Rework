#include "GameStateManager.hpp"
#include "Game.hpp"

void GameStateManager::ChangeState(std::unique_ptr<GameState> new_state, Game* game) {
    if (current_state)
        current_state->Exit(game);
    current_state = std::move(new_state);
    if (current_state)
        current_state->Enter(game);
}

void GameStateManager::HandleInput(Game* game, SDL_Event& e) {
    if (current_state)
        current_state->HandleInput(game, e);
}

void GameStateManager::Update(Game* game, float dt) {
    if (current_state)
        current_state->Update(game, dt);
}

void GameStateManager::Render(Game* game) {
    if (current_state)
        current_state->Render(game);
}