#include "PlayState.hpp"
#include "Game.hpp"

#include <iostream>


int WINDOW_HEIGHT_prime = 1080;
int WINDOW_WIDTH_prime = 1920;

void PlayState::Enter(Game* game) 
{
    std::cout << "[*] Entering PlayState\n";
    game->GetSoundManager().PlayMusic("first_level_song");
}

void PlayState::HandleInput(Game* game, SDL_Event& e) 
{
    game->HandleKeyInput(e, &game->GetPlayer(), game->GetProjectiles(), game->GetRenderCollBoxes());
}

void PlayState::Update(Game* game, float dt) 
{
    game->SpawnEnemies(game->GetEnemies());
    game->HandleCollisions(&game->GetPlayer(), game->GetProjectiles(), game->GetItemManager().GetItemList(), game->GetEnemies(), game->GetRenderCollBoxes());
    game->GetGraphics().BackgroundUpdate(game->GetLoopFlag());
    game->GetPlayer().Update(WINDOW_WIDTH_prime, WINDOW_HEIGHT_prime, game->GetLoopFlag(), dt * 1000, game->GetSoundManager());
    game->GetItemManager().UpdateItemList();
    // ... update projectiles, enemies, etc.
}

void PlayState::Render(Game* game) 
{
    game->GetGraphics().RenderGameItems(&game->GetPlayer(), game->GetProjectiles(), game->GetItemManager(), game->GetEnemies(), game->GetOverlayTextManager(), game->GetRenderCollBoxes());
    game->GetOverlayTextManager().Update();
}

void PlayState::Exit(Game* game) 
{
    std::cout << "[*] Exiting PlayState\n";
}