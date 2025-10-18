#include "Collisions.hpp"
#include <iostream>
#include <algorithm>

// -----------------------------------------------------
// Top-level function to check between any two colliders
// -----------------------------------------------------
bool Collisions::CheckCollision(const Collider& a, const Collider& b, bool print_flag)
{
    bool collided = false;

    if (a.type == ColliderType::RECT && b.type == ColliderType::RECT)
    {
        collided = RectRectCollision(a.rect, b.rect);
    }
    else if (a.type == ColliderType::CIRCLE && b.type == ColliderType::CIRCLE)
    {
        collided = CircleCircleCollision(a.circle.x, a.circle.y, a.circle.r, b.circle.x, b.circle.y, b.circle.r);
    }
    else if (a.type == ColliderType::RECT && b.type == ColliderType::CIRCLE)
    {
        collided = RectCircleCollision(a.rect, b.circle.x, b.circle.y, b.circle.r);
    }
    else if (a.type == ColliderType::CIRCLE && b.type == ColliderType::RECT)
    {
        collided = RectCircleCollision(b.rect, a.circle.x, a.circle.y, a.circle.r);
    }

    if (print_flag)
    {
        std::cout << "Collision check: "
            << (collided ? "COLLIDED" : "NO COLLISION") << std::endl;
    }

    return collided;
}

// -----------------------------------------------------
// Rect-Rect Collision
// -----------------------------------------------------
bool Collisions::RectRectCollision(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
    return (
        rect1.x < rect2.x + rect2.w &&
        rect1.x + rect1.w > rect2.x &&
        rect1.y < rect2.y + rect2.h &&
        rect1.y + rect1.h > rect2.y
        );
}

// -----------------------------------------------------
// Circle-Circle Collision
// -----------------------------------------------------
bool Collisions::CircleCircleCollision(int x1, int y1, int r1, int x2, int y2, int r2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int distanceSq = dx * dx + dy * dy;
    int radiusSum = r1 + r2;
    return distanceSq <= (radiusSum * radiusSum);
}

// -----------------------------------------------------
// Rect-Circle Collision
// -----------------------------------------------------
bool Collisions::RectCircleCollision(const SDL_Rect& rect, int cx, int cy, int cr)
{
    // Find the closest point on the rectangle to the circle’s center
    int closestX = std::max(rect.x, std::min(cx, rect.x + rect.w));
    int closestY = std::max(rect.y, std::min(cy, rect.y + rect.h));

    // Calculate the distance from the circle’s center to this point
    int dx = cx - closestX;
    int dy = cy - closestY;

    return (dx * dx + dy * dy) <= (cr * cr);
}

// -----------------------------------------------------
// Draw a Circle
// -----------------------------------------------------
void Collisions::DrawCircle(SDL_Renderer* renderer, Circle circle)
{
    const int32_t diameter = (circle.r * 2);

    int32_t x = (circle.r - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, circle.x + x, circle.y - y);
        SDL_RenderDrawPoint(renderer, circle.x + x, circle.y + y);
        SDL_RenderDrawPoint(renderer, circle.x - x, circle.y - y);
        SDL_RenderDrawPoint(renderer, circle.x - x, circle.y + y);
        SDL_RenderDrawPoint(renderer, circle.x + y, circle.y - x);
        SDL_RenderDrawPoint(renderer, circle.x + y, circle.y + x);
        SDL_RenderDrawPoint(renderer, circle.x - y, circle.y - x);
        SDL_RenderDrawPoint(renderer, circle.x - y, circle.y + x);

        if (error <= 0) {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void Collisions::ResolveCollision(Collider& mover, const Collider& obstacle, int& out_dx, int& out_dy)
{
    out_dx = 0;
    out_dy = 0;

    if (mover.type == ColliderType::RECT && obstacle.type == ColliderType::RECT)
    {
        SDL_Rect r1 = mover.rect;
        SDL_Rect r2 = obstacle.rect;

        float overlapX = (r1.x + r1.w / 2.0f) - (r2.x + r2.w / 2.0f);
        float overlapY = (r1.y + r1.h / 2.0f) - (r2.y + r2.h / 2.0f);

        float halfWidths = (r1.w + r2.w) / 2.0f;
        float halfHeights = (r1.h + r2.h) / 2.0f;

        float penX = halfWidths - fabsf(overlapX);
        float penY = halfHeights - fabsf(overlapY);

        if (penX > 0 && penY > 0)
        {
            if (penX < penY)
                out_dx = (overlapX > 0) ? (int)penX : (int)-penX;
            else
                out_dy = (overlapY > 0) ? (int)penY : (int)-penY;
        }
    }
    else if (mover.type == ColliderType::CIRCLE && obstacle.type == ColliderType::CIRCLE)
    {
        float dx = mover.circle.x - obstacle.circle.x;
        float dy = mover.circle.y - obstacle.circle.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        float overlap = mover.circle.r + obstacle.circle.r - dist;

        if (overlap > 0.0f && dist > 0.0f)
        {
            float nx = dx / dist; // Normalized direction
            float ny = dy / dist;

            out_dx = static_cast<int>(nx * overlap);
            out_dy = static_cast<int>(ny * overlap);
        }
    }
    else
    {
        // Rect–Circle hybrid resolution (simplified)
        const Collider* rect = (mover.type == ColliderType::RECT) ? &mover : &obstacle;
        const Collider* circle = (mover.type == ColliderType::CIRCLE) ? &mover : &obstacle;

        float cx = (float)circle->circle.x;
        float cy = (float)circle->circle.y;
        float closestX = std::clamp(cx, (float)rect->rect.x, (float)(rect->rect.x + rect->rect.w));
        float closestY = std::clamp(cy, (float)rect->rect.y, (float)(rect->rect.y + rect->rect.h));

        float dx = cx - closestX;
        float dy = cy - closestY;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < circle->circle.r && dist > 0.0f)
        {
            float overlap = circle->circle.r - dist;
            float nx = dx / dist;
            float ny = dy / dist;

            out_dx = static_cast<int>(nx * overlap);
            out_dy = static_cast<int>(ny * overlap);
        }
    }
}

