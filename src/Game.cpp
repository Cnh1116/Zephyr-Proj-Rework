#include "PlayState.hpp"
#include <iostream>
#include "Game.hpp"
#include <iostream>
#include <string>
#include <random>



int PIXEL_SCALE = 4;
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;
float MAX_FPS = 120.0;


int damage_numbers_text_time = 350;
SDL_Color damage_number_color = { 212, 47, 47, 255 };
SDL_Color heal_number_color= { 44, 214, 58, 255 };

Game::Game() // Game constructor acts as my INIT function for the game.
    : graphics_manager(new Graphics("Main Window", WINDOW_WIDTH, WINDOW_HEIGHT, PIXEL_SCALE)), //Screen name, dimensions and pixel scale
      sound_manager(new SoundManager()),
	  animation_manager(new AnimationManager(graphics_manager->GetRenderer())),
      game_over(false),
      item_manager(new ItemManager(animation_manager)),
	  overlay_text_manager(new OverlayTextManager(graphics_manager->GetRenderer(), graphics_manager->GetFont(2))),
      player(Player(PIXEL_SCALE, *animation_manager))
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
    loop_flag = 0;
    game_projectiles.reserve(30);
    enemies.reserve(10);    

    // Play Starting Song
    sound_manager->PlayMusic("first_level_song");
    

    Uint32 last_tick = SDL_GetTicks();
    
    while(false == game_over)
    {
        Uint32 current_tick = SDL_GetTicks();
        Uint32 time_delta = current_tick - last_tick;
        last_tick = current_tick;



		SpawnEnemies(enemies);
        
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  HANDLE Keyboard           ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            HandleKeyInput(event, &player, game_projectiles, render_coll_boxes);
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~
        ~  HANDLE COLLISIONS     ~
        ~~~~~~~~~~~~~~~~~~~~~~~~*/
        HandleCollisions(&player, game_projectiles, item_manager->GetItemList(), enemies, render_coll_boxes);

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  UPDATE Background Clouds                    ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        graphics_manager->BackgroundUpdate(loop_flag);

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  UPDATE Players                              ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        player.Update(WINDOW_WIDTH, WINDOW_HEIGHT, loop_flag, time_delta, *sound_manager);
        if (player.GetCurrentHealth() <= 0)
        {
            game_over = true;
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  UPDATE Projectiles                          ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  UPDATE Enemies                              ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        for (int i = 0; i < enemies.size();)
        {
            enemies.at(i)->Update(&player, game_projectiles, *sound_manager); 

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

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  RENDER Player, Projectiles and Enemies     ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        graphics_manager->RenderGameItems(&player, game_projectiles, *item_manager, enemies, *overlay_text_manager, render_coll_boxes);
        overlay_text_manager->Update();


        /*~~~~~~~~~~~~~~~~~~~~~~~~
        ~  FPS Control Logic     ~
        ~~~~~~~~~~~~~~~~~~~~~~~~*/
        FPSLogic(current_tick);

        loop_flag++;
    }
}

void Game::HandleKeyInput(SDL_Event event, Player* player, std::vector<Projectile*>& game_projectiles, bool &render_coll_boxes)
{
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  HANDLE Discrete           ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
            }
        }

        if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) // LSHIFT//
        {
            if (player->GetPlayerState() == "main" && player->IsDashReady() and (player->GetVX() != 0 || player->GetVY() != 0))
            {
                sound_manager->PlaySound("dash_sound", 60);
                player->UpdatePlayerState("dash");
            }
        }
                   
        if (event.key.keysym.scancode == SDL_SCANCODE_BACKSLASH) // ` //
        {
            std::cout << "[*] ` Key Pressed.\n";
            
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ~  HANDLE Continous EVENTS    ~
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        float dx = 0, dy = 0;
        if (keystates[SDL_SCANCODE_W]) // W //
        {
            dy += -1.0;
        }
        if (keystates[SDL_SCANCODE_A]) // A //
        {
            dx += -1.0;
        }
        if (keystates[SDL_SCANCODE_S]) // S //
        {
            dy += 1.0;
        }
        if (keystates[SDL_SCANCODE_D]) // D //
        {
            dx += 1.0;
        }

        float len = sqrtf(dx * dx + dy * dy);
        if (len > 0.0001f) {
            dx /= len;
            dy /= len;
        }

        if (keystates[SDL_SCANCODE_LCTRL]) // LCTRL //
        {
            dx *= 0.6;
            dy *= 0.6;
        }
        player->SetInput(dx, dy);

        if (keystates[SDL_SCANCODE_LEFT]) // LEFT //
        {
            player->DoSlash(game_projectiles, *sound_manager, true);
        }

        if (keystates[SDL_SCANCODE_RIGHT]) // RIGHT//
        {
            player->DoSlash(game_projectiles, *sound_manager, false);
        }

        if (keystates[SDL_SCANCODE_DOWN]) // DOWN //
        {

            player->ShootSecondaryFire(game_projectiles, *sound_manager, item_manager);
        }


        if (keystates[SDL_SCANCODE_UP] and player->GetPlayerState() != "slash") // UP //
        {
            player->ShootPrimaryFire(game_projectiles, *sound_manager, item_manager);

        }
}

void Game::HandleCollisions(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies, bool render_coll_boxes)
{
    
    // CHECKING COLLISIONS FOR DIFF PROJECTILES
    for (int i = 0; i < game_projectiles.size(); i++)
    {  
        // ITEM COLLECTION
        if ( dynamic_cast<SecondaryFire*>( game_projectiles.at(i) ) )
        {
            if (Collisions::CheckCollision(*game_projectiles.at(i)->GetCollisionShape(), *player->GetSecondaryFireMarkerCollision(), false))
            {
                player->SetSecondaryFireMarkerActive(false);

                bool collided_with_item = false;

                for (int j = 0; j < (*item_list).size(); j++)
                {
                    if (Collisions::CheckCollision((*item_list).at(j).item_cloud_coll_shape, *game_projectiles.at(i)->GetCollisionShape(), false))
                    {
                        collided_with_item = true;
                        if (game_projectiles.at(i)->GetState() == "main")
                        {
                            if((*item_list).at(j).destroyed == false)
                            
                            {
                                player->AddItem(item_list->at(j).name);
                                sound_manager->PlaySound("item_collection_sound", 45);
                            }
                            
                            sound_manager->PlaySound("player_secondary_fire_impact", 50);
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
            // IF THE PROJECTILE IS ACTIVE AND THERE IS COLLISION WITH PLAYER
            if (game_projectiles.at(i)->GetState() == "main" && Collisions::CheckCollision(*game_projectiles.at(i)->GetCollisionShape(), *player->GetCollShape(), false))
            {
                // IF THE PLAYER CAN BE HURT
                if ((player->GetPlayerState() == "main" || player->GetPlayerState() == "dash" || player->GetPlayerState() == "slash"))
                {
                    player->UpdatePlayerState("iframes");
                    std::cout << "[*] Hurting the player. STATE: " << player->GetPlayerState() << std::endl;
                    player->Hurt(game_projectiles.at(i)->damage, *sound_manager);
                    player->UpdatePlayerState("iframes");
                    if (render_coll_boxes)
                    {
                        overlay_text_manager->AddMessage(game_projectiles.at(i)->GetPrintableDamage(),
                            damage_number_color,
                            player->GetDstRect(),
                            damage_numbers_text_time);
                    }
                    
                }

				// PLAY SOUND AND SET STATE TO IMPACT
                if (!game_projectiles.at(i)->GetSoundPlayed())
                {
                    sound_manager->PlaySound(game_projectiles.at(i)->GetSoundEffectImpact(), game_projectiles.at(i)->GetSoundEffectImpactNoise()); // PLAY SOUND PROJECTILE GET SOUND EFFECT KEY
                    game_projectiles.at(i)->SetSoundPlayed(true);
                }

                game_projectiles.at(i)->UpdateState("impact");
                

                if (player->GetPlayerState() == "shield" && Collisions::CheckCollision(*game_projectiles.at(i)->GetCollisionShape(), *player->GetShieldColl(), false))
                {
                    player->DoShield(*sound_manager, game_projectiles.at(i), render_coll_boxes, overlay_text_manager);
                }

                if (player->GetPlayerState() == "iframes" && Collisions::CheckCollision(*game_projectiles.at(i)->GetCollisionShape(), *player->GetCollShape(), false))
                {
                    game_projectiles.at(i)->UpdateState("impact");
                }
            }


        }

        // ENEMY COLLISIONS
        for (int k = 0; k < enemies.size(); k++)
        {
            // PLAYER PROJECTILES HURTING ENEMIES
            if (game_projectiles.at(i)->player_projectile && Collisions::CheckCollision(*game_projectiles.at(i)->GetDstRect(), *enemies.at(k)->GetCollShape(), false))
            {
                if (auto* primary_projectile = dynamic_cast<PrimaryFire*>(game_projectiles.at(i)))
                {
                    if (game_projectiles.at(i)->GetState() == "main")
                    {
                        if (primary_projectile->critical)
                        {
                            sound_manager->PlaySound("player_primary_fire_crit", 90);
                            enemies.at(k)->ChangeHealth(-game_projectiles.at(i)->damage); //NEEDS TO BE CRIT DAMAGE
                            if (render_coll_boxes)
                            {
                                overlay_text_manager->AddMessage(game_projectiles.at(i)->GetPrintableDamage(),
                                    damage_number_color,
                                    enemies.at(k)->GetDstRect(),
                                    damage_numbers_text_time);
                            }
                        }
                        else
                        {
                            sound_manager->PlaySound("player_hit", 60);
                            enemies.at(k)->ChangeHealth(-game_projectiles.at(i)->damage); //NEEDS TO BE CRIT DAMAGE
                            if (render_coll_boxes)
                            {
                                overlay_text_manager->AddMessage(game_projectiles.at(i)->GetPrintableDamage(),
                                    damage_number_color,
                                    enemies.at(k)->GetDstRect(),
                                    damage_numbers_text_time);
                            }
                        }

                        game_projectiles.at(i)->UpdateState("impact");
                        sound_manager->PlaySound("player_secondary_fire_impact", 60);

                        if (enemies.at(k)->GetHealth() <= 0)
                        {
                            enemies.at(k)->UpdateState("death");
                            player->GivePoints(enemies.at(k)->GetPoints());
                        }
                    }
                }
                if (auto* slash_projectile = dynamic_cast<Slash*>(game_projectiles.at(i)))
                {

                    if (slash_projectile->critical and !slash_projectile->impact_sound_played)
                    {
                        sound_manager->PlaySound("jade_drum", 90);
						slash_projectile->SetImpactSoundPlayed(true);
                        if (!slash_projectile->damage_applied)
                        {
                            enemies.at(k)->ChangeHealth(-game_projectiles.at(i)->damage); //NEEDS TO BE CRIT DAMAGE
							slash_projectile->SetDamageApplied(true);
                        }
                        if (render_coll_boxes)
                        {
                            overlay_text_manager->AddMessage(game_projectiles.at(i)->GetPrintableDamage(),
                                damage_number_color,
                                enemies.at(k)->GetDstRect(),
                                damage_numbers_text_time);
                        }
                    }
                    else
                    {
                        sound_manager->PlaySound("player_hit", 60);
                        if (!slash_projectile->damage_applied)
                        {
                            enemies.at(k)->ChangeHealth(-game_projectiles.at(i)->damage); //NEEDS TO BE CRIT DAMAGE
                            slash_projectile->SetDamageApplied(true);
                        }
                        if (render_coll_boxes)
                        {
                            overlay_text_manager->AddMessage(game_projectiles.at(i)->GetPrintableDamage(),
                                damage_number_color,
                                enemies.at(k)->GetDstRect(),
                                damage_numbers_text_time);
                        }
                    }
                    if (!slash_projectile->overlay_added)
                    {
                        enemies.at(k)->AddOverlayAnimation(animation_manager->Get("proj-zephyr-slash", "impact"));
                        slash_projectile->SetOverlayAdded(true);
                    }

                    if (!slash_projectile->sound_played)
                    {
                        sound_manager->PlaySound("player_secondary_fire_impact", 60);
                        slash_projectile->SetSoundPlayed(true);
                    }

                    if (enemies.at(k)->GetHealth() <= 0)
                    {
                        enemies.at(k)->UpdateState("death");
                        player->GivePoints(enemies.at(k)->GetPoints());
                    }

                }
            }
        }
    } // GAME PROJECTILE loop_flag END

    for (auto& enemy : enemies) 
    {
        Collider player_coll = *player->GetCollShape();
        Collider enemy_coll = *enemy->GetCollShape();

        if (Collisions::CheckCollision(enemy_coll, player_coll, false))
        {
            int pushX = 0, pushY = 0;
            Collisions::ResolveCollision(*player->GetCollShape(), *enemy->GetCollShape(), pushX, pushY);

            player->SetPosition(pushX, pushY, WINDOW_WIDTH, WINDOW_HEIGHT);

            // Optional bounce effect:
            if (pushX != 0)
                player->SetVX(player->GetVX() * -0.25f);
            if (pushY != 0)
                player->SetVY(player->GetVY() * -0.25f);

            // Damage logic
            if (player->GetPlayerState() != "iframes")
            {
                int enemy_collision_damage = 20;
                player->Hurt(enemy_collision_damage, *sound_manager);

                if (render_coll_boxes)
                {
                    overlay_text_manager->AddMessage(std::to_string(enemy_collision_damage),
                        damage_number_color, player->GetDstRect(), damage_numbers_text_time);
                }

                player->UpdatePlayerState("iframes");
            }
        }
    }
}

void Game::SpawnEnemies(std::vector<Enemy*> &enemies)
{
    if (enemies.size() == 0 or enemies.size() == 1 or enemies.size() == 2)
    {
        
        // PICK A RANDOM ENEMY
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib_enemy(0, 1000);
        std::cout << "[*] Updating Enemies since size is 0\n";
        int enemy_index = distrib_enemy(gen) % 3;

        

        
        switch (enemy_index)
        {
            case 0:
            {
                std::uniform_int_distribution<> distrib_x(0, graphics_manager->GetScreenWidth());
                int random_location_x = distrib_x(gen);

                std::uniform_int_distribution<> distrib_y(0, 200);
                int random_location_y = distrib_y(gen);
                enemies.emplace_back(new IceCrystal(*animation_manager,
                                                    { random_location_x,
                                                      random_location_y,
                                                      64 * 3,
                                                      64 * 3 }));
                break;
            }

            case 1:
            {
                enemies.emplace_back(new StormCloud(*animation_manager, 
                                                    graphics_manager->GetScreenWidth(), 
                                                    graphics_manager->GetScreenHeight(), 
                                                    player.GetDstRect()->x + (player.GetDstRect()->w / 2), 
                                                    player.GetDstRect()->y + (player.GetDstRect()->h / 2)));
                break;
            }
            case 2:
            {
                std::uniform_int_distribution<> distrib_x(0, graphics_manager->GetScreenWidth() * 0.8);
                int random_location_x = distrib_x(gen);

                std::uniform_int_distribution<> distrib_y(0, graphics_manager->GetScreenHeight() * 0.8);
                int random_location_y = distrib_y(gen);
                
                enemies.emplace_back(new StormGenie(*animation_manager, 
                                                    { random_location_x,
                                                      random_location_y,
                                                      64 * 3,
                                                      64 * 3 }));
                break;
            }
            default:
            {
                std::cout << "[!] ERROR Default hit for enemy switch statement\n";
            }
        }

    }
}

void Game::FPSLogic(Uint32 current_tick)
{
    Uint32 frame_time_ms = SDL_GetTicks() - current_tick;
    if ( (frame_time_ms - current_tick) < ( 1000.0 / MAX_FPS ) )
    {
        SDL_Delay( (1000.0 / MAX_FPS ) - frame_time_ms);
    }
}
void Game::ResetGame()
{
    for (auto* p : game_projectiles)
        delete p;
    game_projectiles.clear();

    for (auto* e : enemies)
        delete e;
    enemies.clear();

    item_manager->GetItemList()->clear();

	player.ResetPlayer(1920, 1080);
	game_over = false;
	loop_flag = 0;
}