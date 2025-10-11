
#include <iostream>
#include "Collisions.hpp"

Collisions::Collisions() {}

Collisions::~Collisions() {}

bool Collisions::RectRectCollision(SDL_Rect* rect_1, SDL_Rect* rect_2, bool print_flag)
{
    if (print_flag)
    {
        std::cout << "X: " << rect_1->x << "Y: " << rect_1->y << "W: " << rect_1->w << "H: " << rect_1->h << " VERSUS "
            << "X: " << rect_2->x << "Y: " << rect_2->y << "W: " << rect_2->w << "H: " << rect_2->h << std::endl;

        std::cout << (rect_1->x < rect_2->x + rect_2->w &&
            rect_1->x + rect_1->w > rect_2->x &&
            rect_1->y < rect_2->y + rect_2->h &&
            rect_1->y + rect_1->h > rect_2->y) << std::endl;
    }
    return (rect_1->x < rect_2->x + rect_2->w &&
        rect_1->x + rect_1->w > rect_2->x &&
        rect_1->y < rect_2->y + rect_2->h &&
        rect_1->y + rect_1->h > rect_2->y);
}

bool Collisions::RectCircleCollision(SDL_Rect* rect_1, int circle_x, int circle_y, int circle_r)
{
    return true;
}
bool Collisions::CircleCircleCollision(int circle1_x, int circle1_y, int circle1_r, int circle2_x, int circle2_y, int circle2_r)
{
    return true;
}
