#include "Graphics.hpp"
#include "Sound.hpp"
#include <vector>
#include <memory>

#include "Enemy.hpp"
#include "Projectiles.hpp"
#include "Player.hpp"
#include "ItemManager.hpp"
#include "AnimationManager.hpp"

class Projectile;



class Game
{
    public:
        Game();
        ~Game();
        void RunGame();
        void HandleKeyInput(SDL_Event event, Player* player, std::vector<Projectile*> &game_projectile, bool &render_coll_boxes);
        void FPSLogic(Uint32 current_tick);

        // Collision Functions
        void HandleCollisions(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies);
        bool RectRectCollision(SDL_Rect* rect_1, SDL_Rect* rect_2, bool print_flag);
        bool RectCircleCollision(SDL_Rect* rect_1, int circle_x, int circle_y, int circle_r);
        bool CircleCircleCollision(int circle1_x, int circle1_y, int circle1_r, int circle2_x, int circle2_y, int circle2_r);

        //Spawning Enemies
        void SpawnEnemies(std::vector<Enemy*>& enemies);
        void UpdateEnemies(std::vector<Enemy*>& enemies);

    
    private:
        Graphics* graphics_manager;
        SoundManager* sound_manager;
        ItemManager* item_manager;
		AnimationManager* animation_manager;
        // player
        // enemy spawner
        // item spawner
        bool game_over;
        Player player;

        int times_X_pressed;
};

