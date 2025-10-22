#include "PlayState.hpp"
#include "Game.hpp"

#include <iostream>


int WINDOW_HEIGHT_prime = 1080;
int WINDOW_WIDTH_prime = 1920;

void PlayState::Enter(Game* game) 
{
    if (first_time)
    {
        std::cout << "[PlayState] Entering gameplay...\n";

        game->GetProjectiles().reserve(30);
        game->GetEnemies().reserve(10);
        game->GetSoundManager().PlayMusic("first_level_song");

        game->SetGameOver(false);
        game->ResetGame();
        first_time = false;
    }
    
}

void PlayState::HandleInput(Game* game) 
{
    game->HandleKeyInput(&game->GetPlayer(), game->GetProjectiles(), game->GetRenderCollBoxes());
}

void PlayState::Update(Game* game, float dt) 
{
    Uint32 current_tick = SDL_GetTicks();
    Uint32 time_delta = current_tick - game->GetLastTick();
    game->SetLastTick(current_tick);

    game->SpawnEnemies(game->GetEnemies());
    game->HandleCollisions(&game->GetPlayer(), game->GetProjectiles(), game->GetItemManager().GetItemList(), game->GetEnemies(), game->GetRenderCollBoxes());

    game->GetGraphics().BackgroundUpdate(game->GetLoopFlag());

    // Player
    game->GetPlayer().Update(WINDOW_WIDTH_prime, WINDOW_HEIGHT_prime, game->GetLoopFlag(), time_delta, game->GetSoundManager());
    if (game->GetPlayer().GetCurrentHealth() <= 0)
        game->Quit();

    // Projectiles
    for (int i = 0; i < game->GetProjectiles().size();)
    {
        game->GetProjectiles()[i]->Update();
        if (game->GetProjectiles()[i]->GetState() == "delete")
        {
            delete game->GetProjectiles()[i];
            game->GetProjectiles().erase(game->GetProjectiles().begin() + i);
        }
        else ++i;
    }

    // Enemies
    for (int i = 0; i < game->GetEnemies().size();)
    {
        game->GetEnemies()[i]->Update(&game->GetPlayer(), game->GetProjectiles(), game->GetSoundManager());
        if (game->GetEnemies()[i]->GetState() == "delete")
        {
            delete game->GetEnemies()[i];
            game->GetEnemies().erase(game->GetEnemies().begin() + i);
        }
        else ++i;
    }

    game->GetItemManager().UpdateItemList();
    game->GetOverlayTextManager().Update();

    game->FPSLogic(current_tick);

    game->SetLoopFlag(game->GetLoopFlag() + 1);
}

void PlayState::Render(Game* game) 
{
    game->GetGraphics().RenderGameItems(&game->GetPlayer(), game->GetProjectiles(), game->GetItemManager(), game->GetEnemies(), game->GetOverlayTextManager(), game->GetRenderCollBoxes());
}

void PlayState::Exit(Game* game) 
{
    std::cout << "[*] Exiting PlayState\n";
}