#include "PauseState.hpp"
#include "PlayState.hpp"
#include "MenuState.hpp"
#include "Game.hpp"

#include <iostream>

enum PauseMenuOptions
{
	RESUME,
	RESTART,
    MAINMENU
};


void PauseState::Enter(Game* game)
{
    std::cout << "[PaudeState] Entering gameplay...\n";
	game->GetSoundManager().PauseSoundChannel(-1);
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
                    current_option = MAINMENU; // wrap to bottom
                else
                    current_option = static_cast<PauseMenuOptions>(current_option - 1);
                break;

            case SDL_SCANCODE_DOWN:
                if (current_option == MAINMENU)
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
                else if (current_option == MAINMENU)
                {
                    std::cout << "[*] Quit selected.\n";
                    game->ResetGame();
                    game->GetGameStateManager().ChangeState(new MenuState(), game);
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
    SDL_Renderer* renderer = game->GetGraphics().GetRenderer();
    SDL_RenderClear(renderer);
	int screen_width = game->GetGraphics().GetScreenWidth();
	int screen_height = game->GetGraphics().GetScreenHeight();


    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderCopy(renderer, screen_texture, nullptr, nullptr);;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect dim = { 0, 0, game->GetGraphics().GetScreenWidth(), game->GetGraphics().GetScreenHeight() };
    SDL_RenderFillRect(renderer, &dim);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    std::string resume_text = "Resume";
    std::string restart_text = "Restart";
    std::string mainmenu_text = "Main Menu";

    SDL_Color normal_color = { 255, 255, 255, 255 };
    SDL_Color highlight_color = { 20, 156, 0, 255 };

    // Calculate positions
    int option_width = 200;
    int option_height = 50;
    int start_y = screen_height / 2 - option_height * 2;

    // Render Resume
    SDL_Color color = (current_option == 0) ? highlight_color : normal_color;
    game->GetGraphics().RenderText(resume_text, { screen_width / 2 - option_width / 2, start_y, option_width, option_height }, color);

    // Render Restart
    color = (current_option == 1) ? highlight_color : normal_color;
    game->GetGraphics().RenderText(restart_text, { screen_width / 2 - option_width / 2, start_y + option_height + 10, option_width, option_height }, color);

    // Render Quit
    color = (current_option == 2) ? highlight_color : normal_color;
    game->GetGraphics().RenderText(mainmenu_text, { screen_width / 2 - option_width / 2, start_y + 2 * (option_height + 10), option_width, option_height }, color);

    // Present the updated renderer
    SDL_RenderPresent(renderer);
}

void PauseState::Exit(Game* game)
{
    if (screen_texture)
    {
        SDL_DestroyTexture(screen_texture);
        screen_texture = nullptr;
    }
	game->GetSoundManager().ResumeSoundChannel(-1);
}