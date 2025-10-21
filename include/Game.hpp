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
        void HandleCollisions(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies, bool render_coll_boxes);
        
        //Spawning Enemies
        void SpawnEnemies(std::vector<Enemy*>& enemies);

        // Getters
        Graphics& GetGraphics() { return *graphics_manager; }
        SoundManager& GetSoundManager() { return *sound_manager; }
        AnimationManager& GetAnimationManager() { return *animation_manager; }
        OverlayTextManager& GetOverlayTextManager() { return *overlay_text_manager; }
        ItemManager& GetItemManager() { return *item_manager; }
        Player& GetPlayer() { return player; }
        std::vector<Projectile*>& GetProjectiles() { return game_projectiles; }
        std::vector<Enemy*>& GetEnemies() { return enemies; }
		bool& GetRenderCollBoxes() { return render_coll_boxes; }
		Uint32 GetLoopFlag() { return loop_flag; }
        void ResetGame();

    
    private:
        // ORDER HERE MATTERS
        Graphics* graphics_manager;
        SoundManager* sound_manager;
        AnimationManager* animation_manager;
		OverlayTextManager* overlay_text_manager;
        ItemManager* item_manager;

        std::vector<Projectile*> game_projectiles;
        std::vector<Enemy*> enemies;
        
        bool game_over;
		bool render_coll_boxes = false;
        Player player;
		Uint32 loop_flag;
};

