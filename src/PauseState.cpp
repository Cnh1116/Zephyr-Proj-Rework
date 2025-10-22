#include "PauseState.hpp"
#include "PlayState.hpp"
#include "Game.hpp"

#include <iostream>

enum PauseMenuOptions
{
	RESUME,
	RESTART,
    QUIT
};


void PauseState::Enter(Game* game)
{
    std::cout << "[PaudeState] Entering gameplay...\n";

}

void PauseState::HandleInput(Game* game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            switch (event.key.keysym.scancode)
            {
            case SDL_SCANCODE_UP:
                if (current_option == RESUME)
                    current_option = QUIT; // wrap to bottom
                else
                    current_option = static_cast<PauseMenuOptions>(current_option - 1);
                break;

            case SDL_SCANCODE_DOWN:
                if (current_option == QUIT)
                    current_option = RESUME; // wrap to top
                else
                    current_option = static_cast<PauseMenuOptions>(current_option + 1);
                break;

            case SDL_SCANCODE_RETURN:
                if (current_option == RESUME)
                {
                    std::cout << "[*] Resume selected.\n";
                    game->GetGameStateManager().ChangeState(game->GetPlayStateInstance(), game);
                    return;
                }
                else if (current_option == RESTART)
                {
                    std::cout << "[*] Restart selected.\n";
                    game->ResetGame();
                    game->GetGameStateManager().ChangeState(game->GetPlayStateInstance(), game);
                    return;
                }
                else if (current_option == QUIT)
                {
                    std::cout << "[*] Quit selected.\n";
                    game->Quit();
                    return;
                }
                break;

            default:
                break;
            }
        }
    }
}

void PauseState::Update(Game* game, float dt)
{
   // Do Nothing
}

void PauseState::Render(Game* game)
{
	game->GetGraphics().RenderPauseMenu(current_option);
}

void PauseState::Exit(Game* game)
{
    std::cout << "[*] Exiting PlayState\n";
}