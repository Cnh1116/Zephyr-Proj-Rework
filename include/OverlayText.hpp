#ifndef OVERLAYTEXT_HPP
#define OVERLAYTEXT_HPP


#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class OverlayText {
public:
    // STATIC TEXT CONSTRUCTOR
    OverlayText(SDL_Renderer* renderer,
        TTF_Font* font,
        const std::string& text,
        SDL_Color color,
        int x,
        int y,
        Uint32 duration_ms);

    // FOR TEXT THAT TRACKS AN ENEMY / PLAYER
    OverlayText(SDL_Renderer* renderer, 
                TTF_Font* font,
                const std::string& text, 
                SDL_Color color,
                const SDL_Rect* target_rect, Uint32 duration_ms);

    ~OverlayText();

    bool IsExpired() const;
    void Render() const;

private:
    SDL_Renderer* renderer;
    SDL_Texture* texture = nullptr;
    const SDL_Rect* follow_rect = nullptr;
    SDL_Rect dest{};
    Uint32 start_time;
    Uint32 duration;
    int offset_y = -20;
};
#endif
