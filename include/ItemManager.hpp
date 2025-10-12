#ifndef ITEMMANAGER_HPP
#define ITEMMANAGER_HPP

#include <SDL.h>
#include <vector>
#include <iostream>

class Graphics;
class Animation;
class AnimationManager;

class ItemManager
{
protected:
    AnimationManager* animation_manager;
    public:
       struct item
        {
            SDL_Rect item_dest_rect;
            SDL_Rect item_cloud_dest_rect;
            SDL_Rect item_cloud_coll_rect;
            std::string name;

            std::vector<std::unique_ptr<Animation>> overlay_animations;
            std::unique_ptr<Animation> item_cloud_animation;
            std::unique_ptr<Animation> current_animation;

            bool destroyed;
            // rarity?
        };

        ItemManager(AnimationManager* animation_manager);
        std::vector<item>* GetItemList();
        void UpdateItemList(); // This is where the logic of when to spawn items occurs.
		void DrawItems(SDL_Renderer* renderer, bool collision_box_flag, int screen_width, int screen_height);

    private:

        Uint32 time_last_item_spawned;
        Uint32 cooldown_item_spawn_ms = 3000;

        std::vector<item> item_list;
};

#endif