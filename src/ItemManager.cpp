#include "ItemManager.hpp"
#include <iostream>
#include "Graphics.hpp"
#include <random>
#include "AnimationManager.hpp"
#include "Animation.hpp"
#include "Collisions.hpp"

ItemManager::ItemManager(AnimationManager* animation_manager)
    : animation_manager(animation_manager)
{
    time_last_item_spawned = 0;
    item_list.reserve(5);
}

std::vector<ItemManager::item>* ItemManager::GetItemList()
{
    return &item_list;
}

void ItemManager::UpdateItemList(int screen_width, int screen_height)
{
    Uint32 current_time = SDL_GetTicks();

    if ( (current_time - time_last_item_spawned) >= cooldown_item_spawn_ms ) //EVERY X MS SPAWN AN ITEM
    {

        // RANDOM X LOCATION
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(static_cast<float>(screen_width) * 0.10, static_cast<float>(screen_width) * 0.90);
        int x_spawn_location = distrib(gen);



        // RANDOM ITEM ID
        std::uniform_int_distribution<> distribution(0, 1000);
        int item_index = distribution(gen) % 2;
        
        
        std::string item_to_spawn;
        switch (item_index)
        {
            case 0: 
            {
                item_to_spawn = "glass_toucan";
                break;
            }
            case 1: 
            {
                item_to_spawn = "garnet_shield";
                break;
            }
        default: std::cout << "[!] Random item index is not a valid range ... Item index produced: " << item_index << std::endl;
        }

        item new_item;

        // LOAD ITEM ANIMATION DATA
        Animation* anim = animation_manager->Get("items", item_to_spawn);

        if (!anim) 
        {
            std::cerr << "[!] Error: Animation for item '" << item_to_spawn << "' not found!" << std::endl;
        }

        new_item.current_animation = std::make_unique<Animation>(*animation_manager->Get("items", item_to_spawn));
        new_item.overlay_animations.emplace_back(std::make_unique<Animation>(*animation_manager->Get("overlays", "heal")));
        new_item.item_cloud_animation = std::make_unique<Animation>(*animation_manager->Get("items", "cloud"));
        new_item.pos_y = 0;
        
        

        // SPAWN LOCATION
        SDL_Rect spawn_location = {     x_spawn_location, 
                                        new_item.pos_y, 
                                        32,
                                        32 };



        SDL_Rect cloud_dest_rect = {    spawn_location.x,
                                        spawn_location.y,
                                        new_item.item_cloud_animation->GetFrameWidth() * new_item.item_cloud_animation->GetScale(),
                                        new_item.item_cloud_animation->GetFrameHeight()* new_item.item_cloud_animation->GetScale() };


        SDL_Rect item_dest_rect = {     cloud_dest_rect.x + (cloud_dest_rect.w / 2) - (new_item.current_animation->GetFrameWidth() * new_item.current_animation->GetScale() / 2),
                                        cloud_dest_rect.y + (cloud_dest_rect.h / 2) - (new_item.current_animation->GetFrameHeight() * new_item.current_animation->GetScale() / 2),
                                        new_item.current_animation->GetFrameWidth() * new_item.current_animation->GetScale(),
                                        new_item.current_animation->GetFrameHeight()* new_item.current_animation->GetScale() };
        
        
        new_item.item_cloud_dest_rect = cloud_dest_rect;
        new_item.item_dest_rect = item_dest_rect;
        new_item.item_cloud_coll_shape.type = ColliderType::CIRCLE;
		new_item.item_cloud_coll_shape.circle.x = cloud_dest_rect.x + cloud_dest_rect.w / 2;
        new_item.item_cloud_coll_shape.circle.y = cloud_dest_rect.y + cloud_dest_rect.h / 2;
        new_item.item_cloud_coll_shape.circle.r = cloud_dest_rect.w / 3;

        new_item.name = item_to_spawn;
        new_item.destroyed = false;

        time_last_item_spawned = current_time;
        item_list.emplace_back(std::move(new_item));
    }


    for (int i = 0; i < item_list.size(); i++)
    {  
        
        item_list.at(i).pos_y += ITEM_SPEED;
        
        item_list.at(i).item_cloud_dest_rect.y = static_cast<int>(item_list.at(i).pos_y);

        item_list.at(i).item_dest_rect.y = item_list.at(i).item_cloud_dest_rect.y + item_list.at(i).item_cloud_dest_rect.h / 2 - (item_list.at(i).item_dest_rect.h / 2);
        
        
        item_list.at(i).item_cloud_coll_shape.circle.y = item_list.at(i).item_dest_rect.y + (item_list.at(i).item_dest_rect.h / 2);


    }
}

void ItemManager::DrawItems(SDL_Renderer* renderer, bool collision_box_flag, int screen_width, int screen_height)
{


    for (int i = 0; i < item_list.size(); i++)
    {
        if (item_list.at(i).item_dest_rect.x >= 0 &&
            item_list.at(i).item_dest_rect.x <= screen_width - item_list.at(i).item_dest_rect.w &&
            item_list.at(i).item_dest_rect.y + item_list.at(i).item_dest_rect.h >= 0 &&
            item_list.at(i).item_dest_rect.y <= screen_height)
        {
            item_list.at(i).item_cloud_animation->Draw(renderer, item_list.at(i).item_cloud_dest_rect);

            if (item_list.at(i).destroyed == false)
                item_list.at(i).current_animation->Draw(renderer, item_list.at(i).item_dest_rect);

            if (collision_box_flag)
            {

                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawRect(renderer, &item_list.at(i).item_cloud_dest_rect);
                SDL_RenderDrawRect(renderer, &item_list.at(i).item_dest_rect);
                
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                if (!item_list.at(i).destroyed)
                    Collisions::DrawCircle(renderer, item_list.at(i).item_cloud_coll_shape.circle);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
            }
        }
        else
        {
            //std::cout << "[*] Need to get rid of this proj\n";
            item_list.erase(item_list.begin() + i);
        }
    }
}
