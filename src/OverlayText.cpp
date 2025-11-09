#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "OverlayText.hpp"


// STATIC TEXT CONSSTRUCTOR
OverlayText::OverlayText(   SDL_Renderer* renderer, 
                            TTF_Font* font,
                            const std::string& text, 
                            SDL_Color color,
                            int x, 
                            int y, 
                            Uint32 duration_ms)
    : renderer(renderer), follow_rect(nullptr),
    start_time(SDL_GetTicks()), duration(duration_ms)

{
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color);
    texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    last_tick = SDL_GetTicks();
    offset_y = -0.04f - (rand() % 10) / 500.0f;
    velocity_y = -0.04f - (rand() % 10) / 500.0f; // slightly random speed
    last_tick = SDL_GetTicks();

    SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);
    dest.x = x - dest.w / 2;
    dest.y = y - dest.h / 2;
}

// FOR TEXT THAT TRACKS AN ENEMY / PLAYER
OverlayText::OverlayText(SDL_Renderer* renderer, TTF_Font* font,
    const std::string& text, SDL_Color color,
    const SDL_Rect* target_rect, Uint32 duration_ms)
    : renderer(renderer), follow_rect(target_rect),
    start_time(SDL_GetTicks()), duration(duration_ms)
{
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color);
    texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    last_tick = SDL_GetTicks();
    offset_y = -0.04f - (rand() % 10) / 500.0f;
    velocity_y = -0.04f - (rand() % 10) / 500.0f; // slightly random speed
    last_tick = SDL_GetTicks();

    SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);
}

void OverlayText::Update() {
    Uint32 now = SDL_GetTicks();
    Uint32 delta = now - last_tick;
    last_tick = now;

    // Move the text vertically
    offset_y_prime += velocity_y * delta;
}


OverlayText::~OverlayText()
{
    if (texture) SDL_DestroyTexture(texture);
}

bool OverlayText::IsExpired() const 
{
    return SDL_GetTicks() - start_time >= duration;
}

void OverlayText::Render() const {
    SDL_Rect render_rect = dest;

    // If tracking a moving object, update position each frame
    if (follow_rect) {
        render_rect.x = follow_rect->x + (follow_rect->w / 2) - (dest.w / 2);
        render_rect.y = follow_rect->y - dest.h + offset_y_prime;
    }

    SDL_RenderCopy(renderer, texture, nullptr, &render_rect);
}
