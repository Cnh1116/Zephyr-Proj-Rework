#include "MenuState.hpp"
#include "PlayState.hpp"
#include "Game.hpp"
#include <iostream>


enum PauseMenuOptions
{
    PLAY,
    OPTIONS,
    QUIT
};

void MenuState::Enter(Game* game)
{
    game->GetSoundManager().StopSoundChannel(-1);
}

void MenuState::HandleInput(Game* game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            switch (event.key.keysym.scancode)
            {
            case SDL_SCANCODE_UP:
                if (current_option == PLAY)
                    current_option = QUIT; // wrap to bottom
                else
                    current_option = static_cast<PauseMenuOptions>(current_option - 1);
                break;

            case SDL_SCANCODE_DOWN:
                if (current_option == QUIT)
                    current_option = PLAY; // wrap to top
                else
                    current_option = static_cast<PauseMenuOptions>(current_option + 1);
                break;

            case SDL_SCANCODE_RETURN:
                if (current_option == PLAY)
                {
                    game->ResetGame();
                    game->GetGameStateManager().ChangeState(game->GetPlayStateInstance(), game);
                    return;
                }
                else if (current_option == QUIT)
                {
                    game->Quit();
                    return;
                }

            default:
                break;
            }
        }
    }
}

void MenuState::Update(Game* game, float dt)
{
    // Do Nothing
}

void MenuState::Render(Game* game)
{
    SDL_Renderer* renderer = game->GetGraphics().GetRenderer();
    SDL_RenderClear(renderer);
    int screen_width = game->GetGraphics().GetScreenWidth();
    int screen_height = game->GetGraphics().GetScreenHeight();


   
    std::string play_text = "Play";
    std::string options_text = "Options";
    std::string quit_text = "Quit";

    SDL_Color normal_color = { 255, 255, 255, 255 };
    SDL_Color highlight_color = { 20, 156, 0, 255 };

    // Calculate positions
    int option_width = 200;
    int option_height = 50;
    int start_y = screen_height / 2 - option_height * 2;

    // Render Resume
    SDL_Color color = (current_option == 0) ? highlight_color : normal_color;
    game->GetGraphics().RenderText(play_text, { screen_width / 2 - option_width / 2, start_y, option_width, option_height }, color);

    // Render Restart
    color = (current_option == 1) ? highlight_color : normal_color;
    game->GetGraphics().RenderText(options_text, { screen_width / 2 - option_width / 2, start_y + option_height + 10, option_width, option_height }, color);

    // Render Quit
    color = (current_option == 2) ? highlight_color : normal_color;
    game->GetGraphics().RenderText(quit_text, { screen_width / 2 - option_width / 2, start_y + 2 * (option_height + 10), option_width, option_height }, color);

    // Present the updated renderer
    SDL_RenderPresent(renderer);
}

void MenuState::Exit(Game* game)
{
    
}