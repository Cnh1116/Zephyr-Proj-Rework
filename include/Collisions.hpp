#ifndef COLLISIONS_HPP
#define COLLISIONS_HPP

#include <SDL.h>


class Collisions
{
public: 
	Collisions();
    ~Collisions();
    static bool RectRectCollision(SDL_Rect* rect_1, SDL_Rect* rect_2, bool print_flag);
    static bool RectCircleCollision(SDL_Rect* rect_1, int circle_x, int circle_y, int circle_r);
    static bool CircleCircleCollision(int circle1_x, int circle1_y, int circle1_r, int circle2_x, int circle2_y, int circle2_r);

};
#endif

