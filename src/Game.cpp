#include "GameStateManager.hpp"
#include "PlayState.hpp"
#include "MenuState.hpp"
#include "PauseState.hpp"
#include <iostream>
#include "Game.hpp"
#include <iostream>
#include <string>
#include <random>



int PIXEL_SCALE = 1;
int LOGICAL_WIDTH = 640;
int LOGICAL_HEIGHT = 360;


int WINDOW_WIDTH = 2560;
int WINDOW_HEIGHT = 1440;

//int WINDOW_WIDTH = 1920;
//int WINDOW_HEIGHT = 1080;

//int WINDOW_WIDTH = 640;
//int WINDOW_HEIGHT = 480;

//int WINDOW_WIDTH = 640;
//int WINDOW_HEIGHT = 360;


float MAX_FPS = 120.0;


int damage_numbers_text_time = 350;
SDL_Color damage_number_color = { 212, 47, 47, 255 };
SDL_Color heal_number_color= { 44, 214, 58, 255 };

Game::Game() // Game constructor acts as my INIT function for the game.
    : graphics_manager(new Graphics("Zephyr the Avenger", LOGICAL_WIDTH, LOGICAL_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, PIXEL_SCALE)), //Screen name, dimensions and pixel scale
      sound_manager(new SoundManager()),
	  animation_manager(new AnimationManager(graphics_manager->GetRenderer())),
      game_over(false),
      item_manager(new ItemManager(animation_manager)),
	  overlay_text_manager(new OverlayTextManager(graphics_manager->GetRenderer(), graphics_manager->GetFont(2))),
      player(Player(PIXEL_SCALE, *animation_manager))
{
    // Load Player Sprite
    player.SetPosition( (LOGICAL_WIDTH /2) - (player.GetDstRect()->w / 2), LOGICAL_HEIGHT - (player.GetDstRect()->h), LOGICAL_WIDTH, LOGICAL_HEIGHT);

    play_state = std::make_unique<PlayState>();
    game_state_manager.ChangeState(new MenuState(), this);
}

Game::~Game()
{
    delete graphics_manager;
    delete sound_manager;
    delete item_manager;
}

void Game::RunGame()
{

    bool running = true;
    Uint32 last_frame = SDL_GetTicks();

    while (running)
    {
        Uint32 now = SDL_GetTicks();
        float deltaTime = (now - last_frame) / 1000.0f;
        last_frame = now;

        game_state_manager.HandleInput(this);
        game_state_manager.Update(this, deltaTime);
        game_state_manager.Render(this);
        if (pause_requested)
        {
            SDL_Renderer* renderer = graphics_manager->GetRenderer();

            SDL_Texture* snapshot = graphics_manager->GetCurrentScreenTexture();

            SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
            SDL_SetRenderTarget(renderer, snapshot);

            // Re-render the current state into the snapshot texture
            game_state_manager.Render(this);

            SDL_SetRenderTarget(renderer, prev_target);

            // Now the snapshot has a copy of the current screen
            this->GetGameStateManager().ChangeState(new PauseState(snapshot), this);
            pause_requested = false;
            continue;
        }

        if (game_over)
            running = false;
    }
}

void Game::HandleKeyInput(Player* player, std::vector<Projectile*>& game_projectiles, bool &render_coll_boxes)
{
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
    
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~  HANDLE Discrete           ~
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {


        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) // ESC //
        {
            std::cout << "[*] Esc Key Pressed. game_over = True\n";
            pause_requested = true;
            return;
            //Quit();
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
                                sound_manager->PlaySound("item_collection_sound", 80);
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
                    // PLAY SOUND AND SET STATE TO IMPACT
                    if (!game_projectiles.at(i)->GetSoundPlayed())
                    {
                        sound_manager->PlaySound(game_projectiles.at(i)->GetSoundEffectImpact(), game_projectiles.at(i)->GetSoundEffectImpactNoise()); // PLAY SOUND PROJECTILE GET SOUND EFFECT KEY
                        game_projectiles.at(i)->SetSoundPlayed(true);
                    }
                    player->UpdatePlayerState("iframes");
                    
                    if (dynamic_cast<LightningStrike*>(game_projectiles.at(i)))
                        player->AddOverlayAnimation(animation_manager->Get("overlays", "lightning_explosion"));
                    if (render_coll_boxes)
                    {
                        overlay_text_manager->AddMessage(game_projectiles.at(i)->GetPrintableDamage(),
                            damage_number_color,
                            player->GetDstRect(),
                            damage_numbers_text_time);
                    }
                    
                }

                game_projectiles.at(i)->UpdateState("impact");
                

                if (player->GetPlayerState() == "shield" && Collisions::CheckCollision(*game_projectiles.at(i)->GetCollisionShape(), *player->GetShieldColl(), false))
                {
                    player->DoShield(*sound_manager, game_projectiles.at(i), render_coll_boxes, overlay_text_manager);
                    sound_manager->PlaySound(game_projectiles.at(i)->GetSoundEffectImpact(), game_projectiles.at(i)->GetSoundEffectImpactNoise()); // PLAY SOUND PROJECTILE GET SOUND EFFECT KEY
                    game_projectiles.at(i)->SetSoundPlayed(true);
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
                            sound_manager->PlaySound(primary_projectile->GetSoundEffectImpact(), 90);
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
                            sound_manager->PlaySound(primary_projectile->GetSoundEffectImpact(), 55);
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
                        if (!slash_projectile->damage_applied)
                        {
                            enemies.at(k)->ChangeHealth(-game_projectiles.at(i)->damage); //NEEDS TO BE CRIT DAMAGE
							slash_projectile->SetDamageApplied(true);
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
                        
                    }
                    if (!slash_projectile->overlay_added)
                    {
                        if (slash_projectile->GetLeftFlag())
                            enemies.at(k)->AddOverlayAnimation(animation_manager->Get("proj-zephyr-slash", "impact"));
                        else
                            enemies.at(k)->AddOverlayAnimation(animation_manager->Get("proj-zephyr-slash", "impact_right"));
                        slash_projectile->SetOverlayAdded(true);
                        if (render_coll_boxes)
                        {
                            overlay_text_manager->AddMessage(game_projectiles.at(i)->GetPrintableDamage(),
                                damage_number_color,
                                enemies.at(k)->GetDstRect(),
                                damage_numbers_text_time);
                        }
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
    
    if (this->GetElapsedTimeSeconds() < 5)
        return;

    if (enemies.size() == 0 or enemies.size() == 1 or enemies.size() == 2)
    //if (enemies.size() == 0)
    {
        
        // PICK A RANDOM ENEMY
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib_enemy(0, 1000);
        std::cout << "[*] Updating Enemies since size is 0\n";
        int enemy_index = distrib_enemy(gen) % 3;

        enemy_index = 0;

        
        switch (enemy_index)
        {
            case 0:
            {
                std::uniform_int_distribution<> distrib_x(0, graphics_manager->GetScreenWidth() * 0.8);
                int random_location_x = distrib_x(gen);

                enemies.emplace_back(new IceCrystal(*animation_manager,
                                                    { random_location_x,
                                                      -64,
                                                      1,
                                                      1}));
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
                                                      1,
                                                      1}));
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

	player.ResetPlayer(graphics_manager->GetScreenWidth(), graphics_manager->GetScreenHeight());
	game_over = false;
	loop_flag = 0;

    this->ResetGameTimer();
}

void Game::Quit()
{
    game_over = true;
    graphics_manager->DeactivateWindow();
    SDL_Quit();
}