#include "Graphics.hpp"
#include "Sound.hpp"
#include <vector>
#include <memory>

#include "Enemy.hpp"
#include "Projectiles.hpp"
#include "Player.hpp"
#include "ItemManager.hpp"
#include "AnimationManager.hpp"
#include "Collisions.hpp"
#include "OverlayTextManager.hpp"

class Projectile;



class Game
{
    public:
        Game();
        ~Game();
        void RunGame();
        void HandleKeyInput(SDL_Event event, Player* player, std::vector<Projectile*> &game_projectile, bool &render_coll_boxes);
        void FPSLogic(Uint32 current_tick);
        void HandleCollisions(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies);
        
        //Spawning Enemies
        void SpawnEnemies(std::vector<Enemy*>& enemies);

    
    private:
        // ORDER HERE MATTERS
        Graphics* graphics_manager;
        SoundManager* sound_manager;
        AnimationManager* animation_manager;
		OverlayTextManager* overlay_text_manager;
        ItemManager* item_manager;
        
		
        // player
        // enemy spawner
        // item spawner
        bool game_over;
        Player player;
};

