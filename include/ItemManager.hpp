#ifndef ITEMMANAGER_HPP
#define ITEMMANAGER_HPP

#include <SDL.h>
#include <vector>
#include <iostream>

class Graphics;

class ItemManager
{
    
    public:
       struct item
        {
            SDL_Rect item_dest_rect;
            SDL_Rect item_cloud_dest_rect;
            SDL_Rect item_cloud_coll_rect;
            std::string name;
            bool destroyed;
            // rarity?
        };

        ItemManager();
        std::vector<item>* GetItemList();
        void UpdateItemList(); // This is where the logic of when to spawn items occurs.
        SDL_Texture* GetItemCloudTexture();

    private:
        std::string item_cloud_png = "/home/monkey-d-luffy/Cpp-Sdl2-Learning/assets/sprites/item-sprites/item-cloud.png";
        SDL_Texture* item_cloud_texture;

        Uint32 time_last_item_spawned;
        Uint32 cooldown_item_spawn_ms = 3000;

        //ITEMS
        SDL_Texture* glass_toucan_texture;
        std::string  glass_toucan_png = "/home/monkey-d-luffy/Cpp-Sdl2-Learning/assets/sprites/item-sprites/glass_toucan4.png";

        std::vector<item> item_list;




 

};

#endif