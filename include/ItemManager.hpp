#ifndef ITEMMANAGER_HPP
#define ITEMMANAGER_HPP

#include <SDL.h>
#include <vector>
#include <iostream>
#include "Collisions.hpp"

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
            float pos_x;
            float pos_y;
            SDL_Rect item_cloud_dest_rect;
            Collider item_cloud_coll_shape = Collider(0,0,0);
            std::string name;

            std::vector<std::unique_ptr<Animation>> overlay_animations;
            std::unique_ptr<Animation> item_cloud_animation;
            std::unique_ptr<Animation> current_animation;

            bool destroyed;
            // rarity?
        };

        ItemManager(AnimationManager* animation_manager);
        std::vector<item>* GetItemList();
        void UpdateItemList(int screen_width, int screen_height); // This is where the logic of when to spawn items occurs.
		void DrawItems(SDL_Renderer* renderer, bool collision_box_flag, int screen_width, int screen_height);

    private:

        Uint32 time_last_item_spawned;
        Uint32 cooldown_item_spawn_ms = 3000;

        float ITEM_SPEED = 0.35f; //MATCH to player.hpp marker item speed, and to secondary_fire.hpp item speed
        std::vector<item> item_list;
};

#endif