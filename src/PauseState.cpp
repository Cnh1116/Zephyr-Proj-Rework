#include "PauseState.hpp"
#include "PlayState.hpp"
#include "Game.hpp"

#include <iostream>




void PauseState::Enter(Game* game)
{
    std::cout << "[PaudeState] Entering gameplay...\n";

}

void PauseState::HandleInput(Game* game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE && !event.key.repeat) // ESC //
        {
            std::cout << "[*] Esc Key Pressed in Pause.";
            game->GetGameStateManager().ChangeState(game->GetPlayStateInstance(), game);
            return;
        }
    }
}

void PauseState::Update(Game* game, float dt)
{
   
}

void PauseState::Render(Game* game)
{

}

void PauseState::Exit(Game* game)
{
    std::cout << "[*] Exiting PlayState\n";
}