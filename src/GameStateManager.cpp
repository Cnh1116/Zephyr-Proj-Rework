#include "GameStateManager.hpp"
#include "Game.hpp"

GameStateManager::GameStateManager() : active_state(nullptr) {}

void GameStateManager::ChangeState(GameState* new_state, Game* game) 
{
    if (active_state) active_state->Exit(game);
        active_state = new_state;
    if (active_state) active_state->Enter(game);
}

void GameStateManager::HandleInput(Game* game) 
{ 
    if (active_state) active_state->HandleInput(game); 
}

void GameStateManager::Update(Game* game, float dt) 
{ 
    if (active_state) active_state->Update(game, dt); 
}

void GameStateManager::Render(Game* game) 
{ 
    if (active_state) active_state->Render(game); 
}