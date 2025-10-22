#include "GameStateManager.hpp"
#include "Graphics.hpp"
#include "Sound.hpp"
#include "PlayState.hpp"
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
        void ResetGame();
        void Quit();
        void HandleKeyInput(Player* player, std::vector<Projectile*> &game_projectile, bool &render_coll_boxes);
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
		GameStateManager& GetGameStateManager() { return game_state_manager; }
        PlayState* GetPlayStateInstance() { return play_state.get(); }

		bool& GetRenderCollBoxes() { return render_coll_boxes; }
		Uint32 GetLoopFlag() { return loop_flag; }
		Uint32 GetLastTick() { return last_tick; }

        // Setters
		void SetGameOver(bool state) { game_over = state; }
        void SetLastTick(Uint32 last_tick) { this->last_tick = last_tick; }
		void SetLoopFlag(Uint32 loop_flag) { this->loop_flag = loop_flag; }

    
    private:
        // ORDER HERE MATTERS
        Graphics* graphics_manager;
        SoundManager* sound_manager;
        AnimationManager* animation_manager;
		OverlayTextManager* overlay_text_manager;
        ItemManager* item_manager;

        std::vector<Projectile*> game_projectiles;
        std::vector<Enemy*> enemies;

		GameStateManager game_state_manager;
        std::unique_ptr<PlayState> play_state;

        
        bool game_over;
		bool render_coll_boxes = false;
        Player player;
		Uint32 loop_flag;
        Uint32 last_tick = 0;
};

