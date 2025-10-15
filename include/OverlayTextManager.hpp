#ifndef OVERLAYTEXT_MANAGER_HPP
#define OVERLAYTEXT_MANAGER_HPP



#include "OverlayText.hpp"
#include <vector>
#include <memory>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class OverlayTextManager {
public:
    OverlayTextManager(SDL_Renderer* renderer, TTF_Font* font);

    void AddMessage(const std::string& text, SDL_Color color, int x, int y, Uint32 duration_ms); //STATIC TEXT
    void AddMessage(const std::string& text, SDL_Color color, const SDL_Rect* target_rect, Uint32 duration_ms); //TRACKING TEXT
    void Update();
    void RenderOverlays() const;

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::vector<std::unique_ptr<OverlayText>> messages;
};
#endif