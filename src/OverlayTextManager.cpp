#include <vector>
#include <memory>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "OverlayTextManager.hpp"

#include <algorithm>

OverlayTextManager::OverlayTextManager(SDL_Renderer* renderer, TTF_Font* font)
    : renderer(renderer), font(font) {}

void OverlayTextManager::AddMessage(const std::string& text, SDL_Color color, int x, int y, Uint32 duration_ms) {
    messages.push_back(std::make_unique<OverlayText>(renderer, font, text, color, x, y, duration_ms));
}

void OverlayTextManager::AddMessage(const std::string& text, SDL_Color color,
    const SDL_Rect* target_rect, Uint32 duration_ms)
{
    messages.push_back(std::make_unique<OverlayText>(renderer, font, text, color, target_rect, duration_ms));
}

void OverlayTextManager::Update() {
    messages.erase(
        std::remove_if(messages.begin(), messages.end(),
            [](const std::unique_ptr<OverlayText>& msg) { return msg->IsExpired(); }),
        messages.end());
}

void OverlayTextManager::RenderOverlays() const {
    for (const auto& msg : messages)
        msg->Render();
}