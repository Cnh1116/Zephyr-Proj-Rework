#include <iostream>
#include "Game.hpp"
#include <iostream>
#include <string>
#include <random>


int PIXEL_SCALE = 4;
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;
float MAX_FPS = 60.0;

Game::Game() // Game constructor acts as my INIT function for the game.
    : graphics_manager(new Graphics("Main Window", WINDOW_WIDTH, WINDOW_HEIGHT, PIXEL_SCALE)), //Screen name, dimensions and pixel scale
      sound_manager(new SoundManager()),
      game_over(false),
      times_X_pressed(0),
      item_manager(new ItemManager()),
      player(Player(graphics_manager, PIXEL_SCALE))
{
    // Load Player Sprite
    player.SetPosition( (WINDOW_WIDTH/2) - (player.GetDstRect()->w / 2), WINDOW_HEIGHT - (player.GetDstRect()->h), WINDOW_WIDTH, WINDOW_HEIGHT );

    
}

Game::~Game()
{
    delete graphics_manager;
    delete sound_manager;
    delete item_manager;
}

void Game::RunGame()
{

    // Game loop_flag Specific Pieces
    long loop_flag = 0;
    std::vector<Projectile*> game_projectiles;
    std::vector<Enemy*> enemies;
    game_projectiles.reserve(30);
    enemies.reserve(10);

    bool render_coll_boxes = false;
    

    // Play Starting Song
    //sound_manager->PlayMusic("first_level_song");
    


    
    while(false == game_over)
    {
        
        
        
        Uint32 current_tick = SDL_GetTicks();

		SpawnEnemies(enemies);
        
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  HANDLE Discrete EVENTS    ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            HandleKeyInput(event, &player, game_projectiles, render_coll_boxes);
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  HANDLE Continous EVENTS    ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        float dx = 0, dy = 0;
        if (keystates[SDL_SCANCODE_W])
        {
            dy = -1.0;
        }
        if (keystates[SDL_SCANCODE_A])
        {
            dx = -1.0;
        }
        if (keystates[SDL_SCANCODE_S])
        {
            dy = 1.0;
        }
        if (keystates[SDL_SCANCODE_D])
        {
            dx = 1.0;
        }

        // If we are moving diagnal, we need to normalize the movement
        if (dx != 0 && dy != 0)
        {
            float length = sqrt(dx * dx + dy * dy);
            dx /= length;
            dy /= length;
        }

        if (keystates[SDL_SCANCODE_LCTRL])
        {
            dx *= 0.6;
            dy *= 0.6;
        }

        if (keystates[SDL_SCANCODE_DOWN]) // DOWN //
        {
            
            if(player.IsFireSecondaryReady())
            {
                player.SetSecondaryFireMarkerActive(true);
                player.SetSecondaryFireMarkerPosition();
                game_projectiles.emplace_back(new SecondaryFire(*player.GetDstRect(),  player.GetSecondaryFireSpeed(), 4));

                for(int i = 0; i < item_manager->GetItemList()->size(); i++)
                {
                    if (RectRectCollision(player.GetSecondaryFireHudColl(), &item_manager->GetItemList()->at(i).item_dest_rect, false))
                    {
                        std::cout << "[*] Player shot an item!\n";
                        player.AddItem(item_manager->GetItemList()->at(i).name);

                        sound_manager->PlaySound("item_collection_sound", 55);
                    }
                }

                sound_manager->PlaySound("player_secondary_fire", 55);
            }
        }


        if (keystates[SDL_SCANCODE_UP]) // UP ARROW //
        {
            
            if(player.IsFirePrimaryReady())
            {
                // IN ORDER TO MAKE DIFF SIZES WORK, PROJECTILES SHOULD GET ENTIRE SDL REC OT PLAYER / ENEMY. Make the 
                // dest rect of the projectile: X: (player.x + player.w / 2) - (projectile.w / 2)
                // Make collision rect of the projectile = dest rec for now

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distrib(0, 100);
                if (distrib(gen) <= player.GetCrit())
                {
                    sound_manager->PlaySound("player_crit", 45);
                    sound_manager->PlaySound("player_primary_fire", 55);
                    game_projectiles.emplace_back(new PrimaryFire(*player.GetDstRect(), 5.0, player.GetBaseDamage(), 2, true));
                }
                else
                {
                    sound_manager->PlaySound("player_primary_fire", 55);
                    game_projectiles.emplace_back(new PrimaryFire(*player.GetDstRect(), 5.0, player.GetBaseDamage(), 2, false));
                }
                
                std::cout << "[*] UP Pressed. \n";
            }
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~
        ~  HANDLE COLLISIONS     ~
        ~~~~~~~~~~~~~~~~~~~~~~~~*/
        HandleCollisions(&player, game_projectiles, item_manager->GetItemList(), enemies);

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  UPDATE Players, Projectiles and Enemies     ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        graphics_manager->BackgroundUpdate(loop_flag);

        player.Update(dx * player.GetSpeed(), dy * player.GetSpeed(), WINDOW_WIDTH, WINDOW_HEIGHT, loop_flag);
        if (player.GetHealth() <= 0)
            game_over = true;

        for (int i = 0; i < game_projectiles.size();)
        {
            game_projectiles.at(i)->Update(); //proj->update() which calles movePRojectile and should ++animation sprite
            if (game_projectiles.at(i)->GetState() == "delete")
            {
                std::cout << "[*] Deleting a projectile.\n";
                delete game_projectiles.at(i);
                game_projectiles.erase(game_projectiles.begin() + i);
            }
            else
            {
                ++i;
            }
        }

        for (int i = 0; i < enemies.size();)
        {
            enemies.at(i)->Update(&player); //proj->update() which calles movePRojectile and should ++animation sprite
            if (enemies.at(i)->IsReadyToAttack())
            {
                if (dynamic_cast<IceCrystal*>(enemies.at(i)))
                    game_projectiles.emplace_back(new IceShard(enemies.at(i)->enemy_dest_rect, 5.0, 3, enemies.at(i)->base_damage));
                
                if (dynamic_cast<StormCloud*>(enemies.at(i)))
                    game_projectiles.emplace_back(new LightningBall(enemies.at(i)->enemy_dest_rect, 7.5, 3, enemies.at(i)->base_damage, (player.GetCollRect()->x + (player.GetCollRect()->w/2)), (player.GetCollRect()->y + (player.GetCollRect()->h / 2))));
                
                if (dynamic_cast<StormGenie*>(enemies.at(i)))
                {
                    const SDL_Rect left_bolt_dst = { 0, enemies.at(i)->enemy_dest_rect.y, enemies.at(i)->enemy_dest_rect.x, 48};
                    const SDL_Rect right_bolt_dst = { enemies.at(i)->enemy_dest_rect.x + enemies.at(i)->enemy_dest_rect.w, enemies.at(i)->enemy_dest_rect.y, (WINDOW_WIDTH - (enemies.at(i)->enemy_dest_rect.x + enemies.at(i)->enemy_dest_rect.w)), 48 };
                    game_projectiles.emplace_back(new LightningStrike(right_bolt_dst, 3, enemies.at(i)->base_damage, true));
                    game_projectiles.emplace_back(new LightningStrike(left_bolt_dst, 3, enemies.at(i)->base_damage, false));
                }
            }

            if (enemies.at(i)->GetState() == "delete")
            {
                std::cout << "[*] Deleting an enemy.\n";
                delete enemies.at(i);
                enemies.erase(enemies.begin() + i);
            }
            else
            {
                ++i;
            }
        }

        item_manager->UpdateItemList();
        //enemies.emplace_back(new PurpleCrystal({ 400,70,64 * 3,64 * 3 }));
        //UpdateEnemies(enemies);
        

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  RENDER Player, Projectiles and Enemies     ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        graphics_manager->RenderGameItems(&player, game_projectiles, item_manager->GetItemList(), enemies, render_coll_boxes);


        /*~~~~~~~~~~~~~~~~~~~~~~~~
        ~  FPS Control Logic     ~
        ~~~~~~~~~~~~~~~~~~~~~~~~*/
        FPSLogic(current_tick);


        loop_flag++;
    }
}

void Game::HandleKeyInput(SDL_Event event, Player* player, std::vector<Projectile*>& game_projectiles, bool &render_coll_boxes) //item_vector pointer to spawn item
{
        
        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) // ESC //
        {
            std::cout << "[*] Esc Key Pressed. game_over = True\n";
            game_over = true;
            graphics_manager->DeactivateWindow();
            SDL_Quit(); 
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_COMMA) // , //
        {
            std::cout << "[*] , Key Pressed.\n";
            render_coll_boxes = !render_coll_boxes;
        }

        if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) // SPACE //
        {
            if (player->GetPlayerState() == "main" && player->IsShieldReady())
            {
                sound_manager->PlaySound("shield_activate", 55);
                player->UpdatePlayerState("shield");
                player->SetShieldLastTimeUsed(SDL_GetTicks());
            }
        }

        if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) // LSHIFT//
        {
            if (player->GetPlayerState() == "main" && player->IsDashReady())
            {
                sound_manager->PlaySound("dash_sound", 60);
                player->UpdatePlayerState("dash");
            }
        }
                   
        if (event.key.keysym.scancode == SDL_SCANCODE_BACKSLASH) // ` //
        {
            std::cout << "[*] ` Key Pressed.\n";
            
        }
}

void Game::HandleCollisions(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies)
{
    
    // CHECKING COLLISIONS FOR DIFF PROJECTILES
    for (int i = 0; i < game_projectiles.size(); i++)
    {  
        // ITEM COLLECTION
        if ( dynamic_cast<SecondaryFire*>( game_projectiles.at(i) ) )
        {
            if (RectRectCollision(game_projectiles.at(i)->GetDstRect(), player->GetSecondaryFireMarkerCollision(), false))
            {
                player->SetSecondaryFireMarkerActive(false);

                bool collided_with_item = false;
                for (int j = 0; j < (*item_list).size(); j++)
                {
                    if (RectRectCollision(&(*item_list).at(j).item_cloud_coll_rect, game_projectiles.at(i)->GetCollisionRect(), false))
                    {
                        collided_with_item = true;
                        if (game_projectiles.at(i)->GetState() == "main")
                        {
                            sound_manager->PlaySound("player_secondary_fire_impact", 45);
                            game_projectiles.at(i)->UpdateState("impact");
                            (*item_list).at(j).destroyed = true;
                        }
                    }
                }
                if (!collided_with_item)
                    game_projectiles.at(i)->UpdateState("delete");  
            }
        }

        // ENEMY PROJECTILES HURTING PLAYER
        if (!game_projectiles.at(i)->player_projectile)
        {
            if (game_projectiles.at(i)->GetState() == "main")
            {
                if ( (player->GetPlayerState() == "main" || player->GetPlayerState() == "dash") && RectRectCollision(game_projectiles.at(i)->GetDstRect(), player->GetCollRect(), false))
                {
                    sound_manager->PlaySound("player_hit", 100);
                    player->UpdatePlayerState("iframes");

                    game_projectiles.at(i)->UpdateState("impact");
                    std::cout << "[*] Hurting the player. STATE: " << player->GetPlayerState() << std::endl;
                    player->ChangeHealth(-game_projectiles.at(i)->damage);
                    sound_manager->PlaySound(game_projectiles.at(i)->GetSoundEffectImpact(), 55); // PLAY SOUND PROJECTILE GET SOUND EFFECT KEY
                }

                if (player->GetPlayerState() == "shield" && RectRectCollision(game_projectiles.at(i)->GetDstRect(), player->GetShieldColl(), false))
                {
                    sound_manager->PlaySound("player_shield_hit", 90);
                    game_projectiles.at(i)->UpdateState("impact");
                    sound_manager->PlaySound(game_projectiles.at(i)->GetSoundEffectImpact(), 25);

                    if (player->GetNumItem("garnet_shield") > 0)
                    {
                        sound_manager->PlaySound("player_heal", 80);
                        player->ChangeHealth(player->GetNumItem("garnet_shield") * 5); // Heal 5HP for every stack of garnet shield
                        player->SetHealingEffectsActive(true);
                    }
                }

                if (player->GetPlayerState() == "iframes" && RectRectCollision(game_projectiles.at(i)->GetDstRect(), player->GetCollRect(), false))
                {
                    game_projectiles.at(i)->UpdateState("impact");
                }
            }


        }

        // ENEMY COLLISIONS
        for (int k = 0; k < enemies.size(); k++)
        {
            // PLAYER PROJECTILES HURTING ENEMIES
            if (game_projectiles.at(i)->player_projectile && RectRectCollision(game_projectiles.at(i)->GetDstRect(), enemies.at(k)->GetCollRect(), false))
            {

                if (auto* primary_projectile = dynamic_cast<PrimaryFire*>(game_projectiles.at(i)))
                {
                    if (primary_projectile->critical)
                    {
                        if (game_projectiles.at(i)->GetState() == "main")
                        {
                            sound_manager->PlaySound("jade_drum", 90);
                            game_projectiles.at(i)->UpdateState("impact");


                            enemies.at(k)->ChangeHealth(-game_projectiles.at(i)->damage);

                            if (enemies.at(k)->GetHealth() <= 0)
                            {
                                enemies.at(k)->UpdateState("death");
                            }
                        }
                    }

                    else
                    {
                        if (game_projectiles.at(i)->GetState() == "main")
                        {
                            sound_manager->PlaySound("player_hit", 60);
                            game_projectiles.at(i)->UpdateState("impact");


                            enemies.at(k)->ChangeHealth(-game_projectiles.at(i)->damage);

                            if (enemies.at(k)->GetHealth() <= 0)
                            {
                                enemies.at(k)->UpdateState("death");
                            }
                        }
                    }
                    
                    
                }
                
            }

            if (RectRectCollision(enemies.at(k)->GetCollRect(), player->GetCollRect(), false) && (player->GetPlayerState() != "iframes"))
            {
                sound_manager->PlaySound("player_hit", 80);
                player->ChangeHealth(-20);
                player->UpdatePlayerState("iframes");
            }
        }
    } // GAME PROJECTILE loop_flag END


}

void Game::SpawnEnemies(std::vector<Enemy*> &enemies)
{
    if (enemies.size() == 0 or enemies.size() == 1)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 1000);
        std::cout << "[*] Updating Enemies since size is 0\n";
        int enemy_index = distrib(gen) % 3;



        switch (enemy_index)
        {
        case 0:
        {
            enemies.emplace_back(new IceCrystal({ 400,70,64 * 3,64 * 3 }));
            break;
        }

        case 1:
        {
            enemies.emplace_back(new StormCloud(graphics_manager->GetScreenWidth(), graphics_manager->GetScreenHeight(), player.GetDstRect()->x + (player.GetDstRect()->w / 2), player.GetDstRect()->y + (player.GetDstRect()->h / 2)));
            break;
        }
        case 2:
        {
            enemies.emplace_back(new StormGenie({ 400,70,64 * 3,64 * 3 }));
            break;
        }
        default:
        {
            std::cout << "[!] ERROR Default hit for enemy switch statement\n";
        }
        }

    }
}

bool Game::RectRectCollision(SDL_Rect* rect_1, SDL_Rect* rect_2, bool print_flag)
{
    if (print_flag)
    {
        std::cout << "X: " << rect_1->x << "Y: " << rect_1->y << "W: " << rect_1->w << "H: " << rect_1->h << " VERSUS " 
            << "X: " << rect_2->x << "Y: " << rect_2->y << "W: " << rect_2->w << "H: " << rect_2->h << std::endl;

        std::cout << (rect_1->x < rect_2->x + rect_2->w &&
            rect_1->x + rect_1->w > rect_2->x &&
            rect_1->y < rect_2->y + rect_2->h &&
            rect_1->y + rect_1->h > rect_2->y) << std::endl;
    }
    return (rect_1->x < rect_2->x + rect_2->w &&
            rect_1->x + rect_1->w > rect_2->x &&
            rect_1->y < rect_2->y + rect_2->h &&
            rect_1->y + rect_1->h > rect_2->y);
}

bool Game::RectCircleCollision(SDL_Rect* rect_1, int circle_x, int circle_y, int circle_r)
{
    return true;
}
bool Game::CircleCircleCollision(int circle1_x, int circle1_y, int circle1_r, int circle2_x, int circle2_y, int circle2_r)
{
    return true;
}

void Game::FPSLogic(Uint32 current_tick)
{
    Uint32 frame_time_ms = SDL_GetTicks() - current_tick;
    if ( (frame_time_ms - current_tick) < ( 1000.0 / MAX_FPS ) )
    {
        SDL_Delay( (1000.0 / MAX_FPS ) - frame_time_ms);
    }
}