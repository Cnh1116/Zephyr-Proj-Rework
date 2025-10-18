#ifndef COLLISIONS_HPP
#define COLLISIONS_HPP

#include <SDL.h>
#include <cmath>

// Defines which type of collider an object uses
enum class ColliderType
{
    RECT,
    CIRCLE
};

struct Circle
{
	int x, y;
	int r;   
};

// Generic collider that can represent either a rectangle or a circle
struct Collider
{
    ColliderType type;

    SDL_Rect rect; // used if type == RECT
    Circle circle;   // used if type == CIRCLE

    // Constructors for convenience
    Collider(SDL_Rect rect_) : type(ColliderType::RECT), rect(rect_), circle({0,0,0}) {}
    Collider(int x, int y, int r) : type(ColliderType::CIRCLE), rect({ 0,0,0,0 }), circle({ x,y,r }) {}};

class Collisions
{
public:
    Collisions() = default;
    ~Collisions() = default;

    static bool CheckCollision(const Collider& a, const Collider& b, bool print_flag = false);
    static void DrawCircle(SDL_Renderer* renderer, Circle circle);
    static void ResolveCollision(Collider& mover, const Collider& obstacle, int& out_dx, int& out_dy);


private:
    static bool RectRectCollision(const SDL_Rect& rect1, const SDL_Rect& rect2);
    static bool CircleCircleCollision(int x1, int y1, int r1, int x2, int y2, int r2);
    static bool RectCircleCollision(const SDL_Rect& rect, int cx, int cy, int cr);
};



#endif // COLLISIONS_HPP
