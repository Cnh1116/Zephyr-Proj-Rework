#include <iostream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <sstream>
#include <random>

#include "Graphics.hpp"
#include "Projectiles.hpp"
#include "ItemManager.hpp"
#include "Enemy.hpp"


class ItemManager;

// Constructor
Graphics::Graphics(const char* title, int width, int height, int scale)
    : window(nullptr), renderer(nullptr), is_shown(false) , clouds1_animation_index(0)
{
    
    screen_width = width;
    screen_height = height;
    pixel_scale = scale;

    
    // BACKGROUND STUFF
    clouds1L_dest = { GenRandomNumber(-1200, -870), 0, 324 * 5, 576 * 5}; //FILE SIZE * arbitrary scale factor
    clouds1R_dest = { GenRandomNumber(1050, 1400), -45, 324 * 6, 576 * 6 };
    clouds2L_dest = { GenRandomNumber(-1100, -870), 0, 324 * 5, 576 * 5 };
    clouds2R_dest = { GenRandomNumber(450, 600),    -45, 324 * 6, 576 * 6 };
    //clouds3L_dest
    //clouds3R_dest = 
    //clouds2_dest = { -450, 700, 576 * 4, 324 * 4 }; //FILE SIZE * arbitrary scale factor
    //clouds3_dest = { screen_height - (324 * 5 / 2), 300, 576 * 5, 324 * 5}; //FILE SIZE * arbitrary scale factor

    cloud1R_speed = 5;
    cloud1L_speed = 6;
    cloud2R_speed = 3;
    cloud2L_speed = 2;
    cloud3R_speed = 2;
    cloud3L_speed = 2;
    
    if (init(title, width, height)) 
    {
        is_shown = true;
    }

    LoadTextures();

    font_1 = TTF_OpenFont("../../assets/fonts/dash-horizon-font/Dashhorizon-eZ5wg.otf",50);
    if (!font_1) 
    {
        std::cout << "[!] font 1 not initialized.";
    }

}

// Destructor
Graphics::~Graphics() 
{
    std::cout << "[*] Cleaning up window./n";

    for (auto& pair : texture_map) 
    {
        SDL_DestroyTexture(pair.second); 
    }
    texture_map.clear();

    if (renderer) { SDL_DestroyRenderer(renderer); }
    if (window) { SDL_DestroyWindow(window); }
    TTF_CloseFont(font_1);
    TTF_Quit();
}

void Graphics::LoadTextures()
{
    // [ LIST OF TEXTURES ]
    // Background
    texture_map["light_blue_background"] = GetTexture("../../assets/sprites/background-sprites/light-blue-background.png");
    texture_map["clouds1_texture"] =  GetTexture("../../assets/sprites/background-sprites/clouds1.png");
    texture_map["clouds2_texture"] = GetTexture("../../assets/sprites/background-sprites/clouds2.png");
    texture_map["clouds3_texture"] = GetTexture("../../assets/sprites/background-sprites/clouds3.png");

    // Player
    texture_map["player_main_texture"] = GetTexture("../../assets/sprites/zephyr-sprites/zephyr-main.png");
    texture_map["player_secondary_fire_hud_texture"] = GetTexture("../../assets/sprites/zephyr-sprites/zephyr-secondary-fire-hud.png");
    texture_map["player_secondary_fire_marker_texture"] =  GetTexture("../../assets/sprites/zephyr-sprites/zephyr-secondary-fire-marker.png");
    texture_map["player_shield"] = GetTexture("../../assets/sprites/zephyr-sprites/zephyr-shield.png");
    texture_map["player_hurt"] = GetTexture("../../assets/sprites/zephyr-sprites/zephyr-iframes.png");
    texture_map["player_heal_effects"] = GetTexture("../../assets/sprites/zephyr-sprites/heal.png");

    // Projectiles
    texture_map["primary_fire"] = GetTexture("../../assets/sprites/projectile-sprites/primary_fire.png");
    texture_map["primary_fire_impact"] = GetTexture("../../assets/sprites/projectile-sprites/primary_fire_impact.png");
    texture_map["primary_fire_crit_impact"] = GetTexture("../../assets/sprites/projectile-sprites/primary_fire_crit_impact.png");
    texture_map["secondary_fire"] = GetTexture("../../assets/sprites/projectile-sprites/secondary_fire.png");
    texture_map["secondary_fire_impact"] = GetTexture("../../assets/sprites/projectile-sprites/secondary_fire_impact.png");
    
    texture_map["ice_shard_conjure"] = GetTexture("../../assets/sprites/projectile-sprites/Ice_Shard_Cast.png");
    texture_map["ice_shard"] = GetTexture("../../assets/sprites/projectile-sprites/Ice_shard.png");
    texture_map["ice_shard_impact"] = GetTexture("../../assets/sprites/projectile-sprites/Ice_Shard_Hit.png");
    texture_map["lightning_ball"] = GetTexture("../../assets/sprites/projectile-sprites/small-spark-Sheet.png");
    texture_map["lightning_ball_impact"] = GetTexture("../../assets/sprites/projectile-sprites/small-spark-impact.png");
    texture_map["lightning_strike_left"] = GetTexture("../../assets/sprites/projectile-sprites/lightning_strike_left.png");
    texture_map["lightning_strike_right"] = GetTexture("../../assets/sprites/projectile-sprites/lightning_strike_right.png");

    // Items
    texture_map["item_cloud"] = GetTexture("../../assets/sprites/item-sprites/item-cloud.png");
    texture_map["glass_toucan"] = GetTexture("../../assets/sprites/item-sprites/glass-toucan.png");
    texture_map["garnet_shield"] = GetTexture("../../assets/sprites/item-sprites/garnet-shield.png");

    // Enemies
    texture_map["purple_crystal_main"] = GetTexture("../../assets/sprites/enemies-sprites/light_blue.png");
    texture_map["purple_crystal_death"] = GetTexture("../../assets/sprites/enemies-sprites/light_blue_destr.png");
    texture_map["storm_cloud_attack"] = GetTexture("../../assets/sprites/enemies-sprites/storm-cloud-attack.png");
    texture_map["storm_cloud_death"] = GetTexture("../../assets/sprites/enemies-sprites/storm-cloud-death.png"); //NEEDS UNIQUE
    texture_map["storm_genie"] = GetTexture("../../assets/sprites/enemies-sprites/storm_genie_sheet.png"); //NEEDS UNIQUE
    
}

void Graphics::HideWindow()
{
    std::cout << "[*] HideWindow() Called";
    is_shown = false;
    SDL_HideWindow(window);  
}

void Graphics::ShowWindow()
{
    std::cout << "[*] ShowWindow() Called";
    is_shown = true;
    SDL_ShowWindow(window);
}

void Graphics::DeactivateWindow()
{
    is_shown = false;
}

// Initialize SDL, create window and renderer
bool Graphics::init(const char* title, int width, int height) 
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) 
    {
        std::cerr << "[!] SDL Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() != 0)
    {
        std::cerr << "[!] TTF Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width, height,
                              SDL_WINDOW_SHOWN);
    if (!window) 
    {
        std::cerr << "[!] Window creation error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) 
    {
        std::cerr << "[!] Renderer creation error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}


SDL_Texture* Graphics::GetTexture(const char* png_path)
{
    SDL_Surface* temp_surface = IMG_Load(png_path);
    SDL_Texture* retreived_texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
    if (NULL == retreived_texture)
    {
        std::cout << "[!] Failed to load texture" << png_path << std::endl;;
        exit(1);
    }
    SDL_FreeSurface(temp_surface);

    return retreived_texture;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void Graphics::RenderGameItems(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies, bool render_coll_boxes)
{
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    

    /*if (NULL == player->GetDstRect()) // ===================================================================PLAYER
    {
        std::cerr << "[!] DstRect for the player is NULL.";
    }*/


    // BACKGROUD //
    if ( 0 != SDL_RenderCopy(renderer, texture_map["light_blue_background"], NULL, NULL) ) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Background failed to render.\n";
    }

    // cloud 1
    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds1_texture"], NULL, &clouds1L_dest, 0.0, NULL, SDL_FLIP_NONE)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds1L failed to render.\n";
    }

    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds1_texture"], NULL, &clouds1R_dest, 0.0, NULL, SDL_FLIP_HORIZONTAL)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds1R failed to render.\n";
    }

    // cloud 2
    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds2L_dest, 0.0, NULL, SDL_FLIP_NONE)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds2L failed to render.\n";
    }

    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds2R_dest, 0.0, NULL, SDL_FLIP_HORIZONTAL)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds2R failed to render.\n";
    }


 

    // cloud 3
    /*if (!cloud3_flipped)
    {
        if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds3_dest, 0.0, NULL, SDL_FLIP_NONE)) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Clouds1 failed to render.\n";
        }
    }
    else
    {
        if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds3_dest, 0.0, NULL, SDL_FLIP_HORIZONTAL)) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Clouds1 failed to render.\n";
        }
    }

    /*if (0 != SDL_RenderCopy(renderer, texture_map["clouds3_texture"], NULL, &clouds3_dest)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds3 failed to render.\n";
    }*/

    // ITEMS //
    for (int i = 0; i < (*item_list).size(); i++)
    {  
        if ((*item_list).at(i).item_dest_rect.x >= 0 && 
        (*item_list).at(i).item_dest_rect.x  <= screen_width - (*item_list).at(i).item_dest_rect.w &&
        (*item_list).at(i).item_dest_rect.y + (*item_list).at(i).item_dest_rect.h >= 0 && 
        (*item_list).at(i).item_dest_rect.y  <= screen_height) 
        {
            if ( 0 != SDL_RenderCopy(renderer, texture_map["item_cloud"], NULL, &(*item_list).at(i).item_cloud_dest_rect)) //Second arg NULL means use whole png.
                {
                    std::cout << "[!] Item failed to render.\n";
                }
            if ((*item_list).at(i).destroyed == false)
                if ( 0 != SDL_RenderCopy(renderer, texture_map[(*item_list).at(i).name], NULL, &(*item_list).at(i).item_dest_rect)) //Second arg NULL means use whole png.
                    {
                        std::cout << "[!] Item failed to render.\n";
                    }

            if (render_coll_boxes)
            {
         
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawRect(renderer, &(*item_list).at(i).item_cloud_dest_rect);
                SDL_RenderDrawRect(renderer, &(*item_list).at(i).item_dest_rect);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &(*item_list).at(i).item_cloud_coll_rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
            }
        }
        else
        {
            //std::cout << "[*] Need to get rid of this proj\n";
            (*item_list).erase((*item_list).begin() + i);
        }
        
    }


    // ENEMIES //
    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies.at(i)->GetDstRect()->x >= 0 &&
            enemies.at(i)->GetDstRect()->x <= screen_width + enemies.at(i)->GetDstRect()->w &&
            enemies.at(i)->GetDstRect()->y + enemies.at(i)->GetDstRect()->h >= 0 &&
            enemies.at(i)->GetDstRect()->y <= screen_height)
        {

            
			enemies.at(i)->Draw(renderer, render_coll_boxes);
            //if (0 != SDL_RenderCopy(renderer, texture_map[enemies.at(i)->GetTextureKey()], enemies.at(i)->GetFrame(), enemies.at(i)->GetDstRect())) //Second arg NULL means use whole png.
            //{
            //    std::cout << "[!] " << "TextureKey: " << enemies.at(i)->GetTextureKey() << std::endl;
            //    std::cout << "[!] " << "TextureKey: " << enemies.at(i)->GetFrame()->x << enemies.at(i)->GetFrame()->y << enemies.at(i)->GetFrame()->w << enemies.at(i)->GetFrame()->h << std::endl;
            //    std::cout << "[!] " << "TextureKey: " << enemies.at(i)->GetDstRect()->x << enemies.at(i)->GetDstRect()->y << enemies.at(i)->GetDstRect()->w << enemies.at(i)->GetDstRect()->h << std::endl;
            //    std::cout << "[!] Enemy failed to render.\n"; 
            //}
            //if (StormCloud* stormCloud = dynamic_cast<StormCloud*>(enemies[i])) 
            //{
            //    SDL_Rect rect;
            //    int size = 5;
            //    rect.x = stormCloud->GetGoalX() - size / 2; // Center the point
            //    rect.y = stormCloud->GetGoalY() - size / 2; // Center the point
            //    rect.w = size;         // Width of the rectangle
            //    rect.h = size;
            //    // Now it's safe to call the special function
            //    if (render_coll_boxes)
            //    {
            //        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            //        if (0 != SDL_RenderFillRect(renderer, &rect))
            //        {
            //            std::cout << "[*] Error rendering point storm cloud goal point ...\n";
            //        }
            //        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            //    }
            //}

            /*if (render_coll_boxes)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(renderer, enemies.at(i)->GetCollRect());
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }*/
        }
        else
        {
            enemies.at(i)->UpdateState("delete");
        }
    }
   

    // PROJECTILES //
    for (int i = 0; i < game_projectiles.size(); i++)
    {
        // IF PROJ IS INSIDE SCREEN
        if (game_projectiles.at(i)->GetDstRect()->x >= 0 && 
        game_projectiles.at(i)->GetDstRect()->x  <= screen_width - game_projectiles.at(i)->GetDstRect()->w &&
        game_projectiles.at(i)->GetDstRect()->y + game_projectiles.at(i)->GetDstRect()->h >= 0 && 
        game_projectiles.at(i)->GetDstRect()->y  <= screen_height) 
        {   

            if (0 != SDL_RenderCopy(renderer, texture_map[game_projectiles.at(i)->GetTextureKey()], game_projectiles.at(i)->GetFrame(), game_projectiles.at(i)->GetDstRect())) //Second arg NULL means use whole png.
            {
                std::cout << "[!] Proj failed to render.\n";
                std::cout << "[!] " << game_projectiles.at(i)->GetFrame()->x << game_projectiles.at(i)->GetFrame()->y << game_projectiles.at(i)->GetFrame()->w << game_projectiles.at(i)->GetFrame()->h << std::endl;
                std::cout << "[!] Texture Key: " << game_projectiles.at(i)->GetTextureKey() << std::endl;
            }

            if (render_coll_boxes)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(renderer, game_projectiles.at(i)->GetCollisionRect());
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawRect(renderer, game_projectiles.at(i)->GetDstRect());
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
        }
        // If they're not in the screen delete them
        else
        {
            game_projectiles.at(i)->UpdateState("delete");
        }
        
    }

    player->Draw(renderer, render_coll_boxes);

    // PLAYER // // ===================================================================PLAYER/*
    /*
                    // Main Player Render
    if (player->GetPlayerState() == "iframes")
    {
        if (0 != SDL_RenderCopy(renderer, texture_map["player_hurt"], player->GetFrame(), player->GetDstRect())) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Player iframes failed to render.\n";
        }
    }
    else
    {
        if (0 != SDL_RenderCopy(renderer, texture_map["player_main_texture"], player->GetFrame(), player->GetDstRect())) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Player failed to render.\n";
        }
    }
    
                    // Render the secondary fire hud
    if ( 0 != SDL_RenderCopy(renderer, texture_map["player_secondary_fire_hud_texture"], NULL, player->GetSecondaryFirePosition())) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Secondary Fire HUD failed to render.\n";
    }
                    // If sec. fire marker is active, render it
    if (player->IsSecondaryFireMarkerActive())
    {
        SDL_RenderCopy(renderer, texture_map["player_secondary_fire_marker_texture"], NULL, player->GetSecondaryFireMarkerPosition());
    }
                    // If shield is active render it
    if (player->GetPlayerState() == "shield")
    {
        SDL_RenderCopy(renderer, texture_map["player_shield"], player->GetShieldFrame(), player->GetShieldDstRect());
    }

                // Render Effects
    if (player->IsHealingEffectsActive())
    {
        SDL_RenderCopy(renderer, texture_map["player_heal_effects"], player->GetHealEffectsFrame(), player->GetDstRect());
    }

    if (player->IsShieldEffectsActive())
    {
        SDL_Rect shield_effect_dest = { (player->GetDstRect()->x + player->GetDstRect()->w / 4),((player->GetDstRect()->y + player->GetDstRect()->h / 4)),(player->GetDstRect()->w / 2),((player->GetDstRect()->h / 2)) };
        SDL_RenderCopy(renderer, texture_map["garnet_shield"], player->GetShieldEffectsFrame(), &shield_effect_dest);
    }

    if (render_coll_boxes)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
        SDL_RenderDrawRect(renderer, player->GetCollRect());
        SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
        SDL_RenderDrawRect(renderer, player->GetShieldColl());
        SDL_RenderDrawRect(renderer, player->GetSecondaryFireHudColl());
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer, player->GetSecondaryFireMarkerCollision());

        //SDL_RenderDrawRect(renderer, player->GetSecondaryFirePosition());
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
        SDL_RenderDrawRect(renderer, player->GetDstRect());

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    }*/

    // PLAYER UI //
    RenderPlayerText(player);



    SDL_RenderPresent(renderer);

    // INCREMENT FRAMES //
    // Projectiles
    for (int i = 0; i < game_projectiles.size(); i++)
    {
        
        if (IsFrameDone(game_projectiles.at(i)->frame_time_ms_ms, game_projectiles.at(i)->last_frame_time_ms))
        {
            game_projectiles.at(i)->last_frame_time_ms = SDL_GetTicks();
            if (game_projectiles.at(i)->animation_replayable)
            {
                if (game_projectiles.at(i)->current_frame_index == game_projectiles.at(i)->NumOfFrames() - 1)
                    game_projectiles.at(i)->current_frame_index = 0;
                else
                    game_projectiles.at(i)->AdvanceFrame();
            }

            else
            {
                if (game_projectiles.at(i)->current_frame_index < game_projectiles.at(i)->NumOfFrames() - 1)
                    game_projectiles.at(i)->AdvanceFrame();
            }
        }
    }
    //Items
    for (int i = 0; i < (*item_list).size(); i++)
    {

        //if ((*item_list).at(i).item_dest_rect.x >= 0 &&
    }

    // NEEDS TO BE IN PLAYER UPDATE METHOD
    /*
    if (IsFrameDone(player->GetFrameTime(), player->GetLastFrameStart()))
    {
        player->SetLastFrameTime(SDL_GetTicks());
        /*if (game_projectiles.at(i)->animation_replayable)
        {
            if (game_projectiles.at(i)->current_frame_index == game_projectiles.at(i)->NumOfFrames() - 1)
                game_projectiles.at(i)->current_frame_index = 0;
            else
                game_projectiles.at(i)->AdvanceFrame();
        }

        //else
        //{
        if (player->GetFrameIndex() < player->NumOfFrames() - 1)
            player->AdvanceFrame();
        else
            player->SetFrameIndex(0);
        //std::cout << "[*] Player frame index: " << player->GetFrameIndex() << std::endl;
        //}
    }
    // NEEDS TO BE IN PLATER UPDATE METHODS (EFFECTS VECTOR ?)
    if ( player->GetPlayerState() == "shield" && IsFrameDone(player->GetShieldFrameTime(), player->GetLastShieldFrameStart()))
    {
        std::cout << "[*] Shield Frame is done !\n";
        player->AdvanceShieldFrame();
        player->SetShieldLastFrameTime(SDL_GetTicks());
    }
    // NEEDS TP BE IN PLAYER UPDATE METHOD
    if (player->GetPlayerState() == "iframes" && IsFrameDone(player->GetIframeTime(), player->GetLastIFrameStart()))
    {
        player->AdvanceIFrame();
        player->SetLastIFrameTime(SDL_GetTicks());
    }
    // NEEDS TO BE IN PLAYER UPDATE METHOD (EFFECTS VECTOR ?)
    if (player->IsHealingEffectsActive())
    {
        if (IsFrameDone(player->GetHealEffectsFrameTime(), player->GetHealEffectsLastFrameTime()))
        {
            player->SetHealEffectsLastFrameTime(SDL_GetTicks());

            if (player->GetHealEffectsFrameIndex() < player->NumHealEffectsFrames() - 1)
                player->AdvanceHealEffectFrame();
            else
            {
                player->SetHealingEffectsActive(false);
                player->SetHealEffectsFrame(0);
            }

        }
    }
    // NEEDS TO BE IN PLAYER UPDATE METHOD
    if (player->IsShieldEffectsActive())
    {
        if (IsFrameDone(player->GetShieldReadyEffectsFrameTime(), player->GetShieldReadyEffectsLastFrameTime()))
        {
            player->SetShieldReadyEffectsLastFrameTime(SDL_GetTicks());

            if (player->GetShieldReadyEffectsFrameIndex() < player->NumShieldReadyEffectsFrames() - 1)
                player->AdvanceShieldReadyEffectFrame();
            else
            {
                player->SetShieldReadyEffectsActive(false);
                player->SetShieldReadyEffectsFrame(0);
            }

        }
    }*/

    //// NEEDS TO BE IN ENEMY UPDATE METHOD
    //for (int i = 0; i < enemies.size(); i++)
    //{
    //    if (IsFrameDone(enemies.at(i)->GetFrameTime(), enemies.at(i)->GetLastFrameStart()))
    //    {
    //        enemies.at(i)->SetLastFrameTime(SDL_GetTicks());

    //        if (enemies.at(i)->GetFrameIndex() < enemies.at(i)->NumOfFrames() - 1)
    //            enemies.at(i)->AdvanceFrame();
    //        else
    //            enemies.at(i)->SetFrameIndex(0);
    //    }
    //}

}

void Graphics::RenderText( const std::string& text, const SDL_Rect& rect, SDL_Color color)
{    
    SDL_Surface* surface = TTF_RenderText_Solid(font_1, text.c_str(), color);
    if (!surface) 
    {
        std::cout << "[!] Failed to create text surface: " << TTF_GetError();
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);



    if (!texture) 
    {
        std::cout << "[!] Failed to create text texture: " << SDL_GetError();
        return;
    }

    if (SDL_RenderCopy(renderer, texture, nullptr, &rect) != 0) 
    {
        std::cout << "[!] Failed to render text texture: " << SDL_GetError();
    }

    // Clean up the texture
    SDL_DestroyTexture(texture); 
}

int Graphics::GetScreenWidth()
{
    return screen_width;
}

int Graphics::GetScreenHeight()
{
    return screen_height;
}

bool Graphics::IsFrameDone(Uint32 frame_time_ms_ms, Uint32 last_frame_start)
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - last_frame_start) >= frame_time_ms_ms)
    {
        return(true);
    }

    else
    {
        //std::cout << "[*] Frame is not done\n";
        return(false);
    }
}

void Graphics::BackgroundUpdate(Uint32 loop_flag)
{
    // CLOUD 1
    
    if (clouds1L_dest.y >= (screen_height)) 
    { 
        std::cout << "[*] loop_flaging cloud 1L back to the top\n";
        clouds1L_dest.x = GenRandomNumber(-1200, -870);; //INSTEAD OF ALWAYS AT ORIGIN, 
        clouds1L_dest.y = 0 - (clouds1L_dest.h / 2);
        cloud1L_speed = GenRandomNumber(4, 8);
    }

    if (clouds1R_dest.y >= (screen_height))
    {
        std::cout << "[*] loop_flaging cloud 1R back to the top\n";
        clouds1R_dest.x = GenRandomNumber(1050, 1400); //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        clouds1R_dest.y = 0 - (clouds1R_dest.h / 2);
        cloud1R_speed = GenRandomNumber(4, 8);
    }
    if (loop_flag % cloud1L_speed == 0)
    {
        clouds1L_dest.y += 1;
    }

    if (loop_flag % cloud1R_speed == 0)
    {
    
        clouds1R_dest.y += 1;
    }

    // CLOUD 2
    if (clouds2L_dest.y >= (screen_height))
    {
        std::cout << "[*] loop_flaging cloud 1L back to the top\n";
        clouds2L_dest.x = GenRandomNumber(-1200, -870);; //INSTEAD OF ALWAYS AT ORIGIN, 
        clouds2L_dest.y = 0 - (clouds2L_dest.h);
        cloud1L_speed = GenRandomNumber(2, 3);
    }

    if (clouds2R_dest.y >= (screen_height))
    {
        std::cout << "[*] loop_flaging cloud 1R back to the top\n";
        clouds2R_dest.x = GenRandomNumber(450, 600); //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        clouds2R_dest.y = 0 - (clouds2R_dest.h);
        cloud1R_speed = GenRandomNumber(1, 3);
    }

    if (loop_flag % cloud2L_speed == 0)
    {
        clouds2L_dest.y += 1;
    }

    if (loop_flag % cloud2R_speed == 0)
    {

        clouds2R_dest.y += 1;
    }

    
}

void Graphics::RenderPlayerText(Player* player)
{
    // Weird logic to make the crit string be 2 decimal places.
    float crit_percent = player->GetCrit();
    std::stringstream string_stream;
    string_stream.precision(2);
    string_stream << std::fixed << crit_percent;
    std::string crit_string = string_stream.str();
    

    std::string player_pos = std::string("X: ") + std::to_string(player->GetDstRect()->x) + std::string(" Y: ") + std::to_string(player->GetDstRect()->y);
    std::string player_damage = std::string("Base Damage ") + std::to_string(static_cast<int>(player->GetBaseDamage()));
    std::string player_crit = std::string("Crit Chance ") + crit_string + "%";
    std::string player_health = std::string("Health ") + std::to_string(static_cast<int>(player->GetHealth()));
    std::string player_speed = std::string("Speed ") + std::to_string(static_cast<int>(player->GetSpeed()));
    std::string state = std::string("State: ") + player->GetPlayerState();
    
    std::string points = std::string("Points: xxx");
    
    
    RenderText(player_pos, { screen_width - static_cast<int>(player_pos.length()) * 12, screen_height - 30, static_cast<int>(player_pos.length()) * 12, 30 }, { 0,0,0,0 });
    RenderText(player_damage, { screen_width - static_cast<int>(player_damage.length()) * 12, screen_height - 60, static_cast<int>(player_damage.length()) * 12, 30 }, { 0,0,0,0 });
    RenderText(player_crit, { screen_width - static_cast<int>(player_crit.length()) * 12, screen_height - 90, static_cast<int>(player_crit.length()) * 12, 30 }, { 0,0,0,0 });
    RenderText(player_health, { screen_width - static_cast<int>(player_health.length()) * 12, screen_height - 120, static_cast<int>(player_health.length()) * 12, 30 }, { 0,0,0,0 });
    RenderText(player_speed, { screen_width - static_cast<int>(player_speed.length()) * 12, screen_height - 150, static_cast<int>(player_speed.length()) * 12, 30 }, { 0,0,0,0 });
    RenderText(state, { screen_width - static_cast<int>(state.length()) * 12, screen_height - 180, static_cast<int>(state.length()) * 12, 30 }, { 0,0,0,0 });
    RenderText(points, { screen_width / 2 - static_cast<int>(state.length()) * 12, 0, static_cast<int>(state.length()) * 12, 30 }, { 0,0,0,0 });
}

SDL_Renderer* Graphics::GetRenderer()
{
    return renderer;
}

int Graphics::GenRandomNumber(int low, int high)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(low, high);
    return (distrib(gen));
}

