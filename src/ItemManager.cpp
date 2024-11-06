#include "ItemManager.hpp"
#include <iostream>
#include "Graphics.hpp"
#include <random>



ItemManager::ItemManager()
{
    time_last_item_spawned = 0;
    item_list.reserve(5);
}

std::vector<ItemManager::item>* ItemManager::GetItemList()
{
    return &item_list;
}

void ItemManager::UpdateItemList() // This is where the logic of when to spawn items occurs and moving them downwards.
{
    Uint32 current_time = SDL_GetTicks();

    if ( (current_time - time_last_item_spawned) >= cooldown_item_spawn_ms ) //EVERY X MS SPAWN AN ITEM
    {

        // RANDOM X LOCATION
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 2000);
        int x_spawn_location = distrib(gen);

        std::uniform_int_distribution<> distribution(0, 1000);
        int item_index = distribution(gen) % 2;
        
        SDL_Rect spawn_location = { x_spawn_location, 0, 32, 32}; //32 IS BASE SPRITE SIZE

        SDL_Rect tmp_item_dest_rect = {spawn_location.x, spawn_location.y, spawn_location.w * 2, spawn_location.h * 2};
        SDL_Rect tmp_item_cloud_dest_rect = {tmp_item_dest_rect.x - (tmp_item_dest_rect.w / 2), tmp_item_dest_rect.y - (tmp_item_dest_rect.h / 2), tmp_item_dest_rect.w * 2, tmp_item_dest_rect.h * 2};
        SDL_Rect tmp_item_cloud_coll_rect = { tmp_item_cloud_dest_rect.x + (tmp_item_cloud_dest_rect.w / 2) - (64 / 2), tmp_item_cloud_dest_rect.y + (tmp_item_cloud_dest_rect.h / 2) - (64 / 2) , 64, 64 };

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
        item_list.push_back({tmp_item_dest_rect, tmp_item_cloud_dest_rect, tmp_item_cloud_coll_rect, item_to_spawn});
        
        
        
        time_last_item_spawned = current_time;
    }

    else
    {
        //std::cout << "[*] Waiting to spawn an item\n";
    }

    for (int i = 0; i < item_list.size(); i++)
    {  
        item_list.at(i).item_dest_rect.y += 1;
        item_list.at(i).item_cloud_dest_rect.y += 1;
        item_list.at(i).item_cloud_coll_rect.y += 1;
    }
}

SDL_Texture* ItemManager::GetItemCloudTexture()
{
    return item_cloud_texture;
}
