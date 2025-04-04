#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <ctime>
using namespace std;

const int groundLevel = 200;
// shortcut
void setImagePosition(SDL_FRect& imagePosition, int const& imgW, int const imgH, int const& imgX, int const& imgY) {
    imagePosition.h = imgH;
    imagePosition.w = imgW;
    imagePosition.x = imgX;
    imagePosition.y = imgY;
}
// get the window center position for a SDL_FRect
double getGroundBlockPosY(SDL_FRect &image, double image_HEIGHT) {
    return groundLevel + image_HEIGHT - image_HEIGHT / 5;
}

bool checkCollision(SDL_FRect& a, SDL_FRect& b) {
    // Check if the rectangles overlap
    return (a.x < b.x + b.w && // a's left < b's right
            a.x + a.w > b.x && // a's right > b's left
            a.y < b.y + b.h && // a's top < b's bottom
            a.y + a.h > b.y);  // a's bottom > b's top
}

// VARIABLE INITIALIZATIONS !!! 
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 360;
const int player_FRAME_WIDTH = 16;
const int player_FRAME_HEIGHT = 26;
const int player_TOTAL_FRAMES = 9;
const int ANIMATION_SPEED = 100;
const int polyT_WIDTH = 64;
const int polyT_HEIGHT = 64;
bool LOST = false;


const double gravity = 1000.f;
double jumpForce = 300.f;
double player_VELOCITY_Y = 0;
double dripstone_VELOCITY_Y = 0;
bool isGroundedDripstone = false;
bool isGrounded = true;
int player_SPEED = 50.f;
SDL_FlipMode playerFacing;


float deltaTime = 0;
Uint32 lastTime = SDL_GetTicks();
Uint32 currentTime = 0;


int player_POS_X = 300;
int player_POS_Y = 200;
int player_DIRECTION_X = 0;
float bg_POS_X = 0;
float bg_POS_Y = 0;
int dripstone_POS_X = 0;
int dripstone_POS_Y = 0;
int playerLives = 3;

SDL_FRect playerFrame;
SDL_FRect playerPortion;
SDL_FRect bg_POS,
gem_block_POS,
wood_block_POS,
grass_block_POS,
dirt_block_POS,
metal_block_POS,
dripstone_DOWN_POS;


float getCenterX(SDL_FRect& imagePosition, int const& winW) {
    return (winW / 2) - (imagePosition.w / 2);
}

float getCenterY(SDL_FRect& imagePosition, int const& winH) {
    return (winH / 2) - (imagePosition.h / 2);
}





int main(int argc, char* argv[]) {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("Dripstone Dodger", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }


    SDL_Texture* bgTexture_CASTLE = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("background_castle.bmp"));
    SDL_Texture* playerT = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("player.bmp"));
    SDL_Texture* gemTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("purple_gem.bmp"));
    SDL_Texture* grassBlockTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("grass_block.bmp"));
    SDL_Texture* dirtBlockTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("dirt_block.bmp"));
    SDL_Texture* woodBlockTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("wood_block.bmp"));
    SDL_Texture* metalBlockTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("metal_block.bmp"));
    SDL_Texture* bgTexture_PRAIRIE = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("bg_prairie.bmp"));
    SDL_Texture* dripstone_DOWNTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("dripstone_DOWN.bmp"));
    SDL_Texture* GAME_BANNERTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("DripstoneDodgerBanner.bmp"));
    SDL_Texture* GAME_LOST_BANNERTexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("LOSE_SCREEN.bmp"));

    SDL_FPoint playerCenterPt;
  

 
    int currentFrame = 0;
    Uint32 lastFrameTime = SDL_GetTicks();
    bool running = false;
    
    const bool* kbdState = SDL_GetKeyboardState(NULL);
    while (true) {
        SDL_PumpEvents(); // Update the event state
        SDL_RenderTexture(renderer, GAME_BANNERTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        if (kbdState[SDL_SCANCODE_SPACE]) {
            running = true;
            SDL_DestroyTexture(GAME_BANNERTexture); // Clean up the banner texture
            break;
        }
    }

loseLabel:
    while (LOST) {
        SDL_PumpEvents(); 
        SDL_RenderTexture(renderer, GAME_LOST_BANNERTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        if (kbdState[SDL_SCANCODE_SPACE]) {
            running = true;
            playerLives = 4;
            SDL_DestroyTexture(GAME_BANNERTexture); 
            break;
        }
    }

   
    srand(time(0));
    
    while (running) {

        

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } 
        }

        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.f;
        lastTime = currentTime;

        const bool *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_RIGHT]) {
            player_POS_X += 1;
            playerFrame.x += 1;
            playerFacing = SDL_FLIP_NONE;
            
        }
        if (state[SDL_SCANCODE_LEFT]) {
            player_POS_X -= 1;
            playerFrame.x += -1;
            playerFacing = SDL_FLIP_HORIZONTAL;
        }
        if (state[SDL_SCANCODE_UP] && isGrounded) {
            player_VELOCITY_Y = -jumpForce; // Negative force -> upward direction!
            isGrounded = false; // player is now in the air
        }

        // Applying gravity
        if (!isGrounded) {
            player_VELOCITY_Y += gravity * deltaTime; // Apply gravity to vertical velocity
            player_POS_Y += player_VELOCITY_Y * deltaTime; // Update player pos based on velocity

            // Check if the player has landed
            if (player_POS_Y >= groundLevel) {
                player_POS_Y = groundLevel; // get back on ground
                player_VELOCITY_Y = 0; // vertical velocity reset
                isGrounded = true; // player is now on the ground
            }

        }
        if (!isGroundedDripstone) {
            dripstone_VELOCITY_Y += gravity * deltaTime; // Apply gravity to the dripstone to fall
            dripstone_POS_Y += dripstone_VELOCITY_Y * deltaTime; // Update dripstone pos based on velocity
            
            if (dripstone_POS_Y >= groundLevel) {
                dripstone_POS_Y = groundLevel;
                dripstone_VELOCITY_Y = 0;
                isGroundedDripstone = true;
            }
        }
        else {
            dripstone_POS_Y = 0;
            isGroundedDripstone = false;
            dripstone_POS_X = rand() % 540 + 100;
        }
        if (checkCollision(playerFrame, dripstone_DOWN_POS)) {
            cout << "Collision detected!" << endl;
            dripstone_POS_Y = 0;
            dripstone_POS_X = rand() % 540 + 100;
            dripstone_VELOCITY_Y = 0;
            playerLives -= 1;
            cout << "Lives: " << playerLives << endl;
            if (playerLives == 0) {
                running = false;
                LOST = true;
                goto loseLabel;
            }
        }

        if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_RIGHT]) {
            currentTime = SDL_GetTicks();
            if (currentTime - lastFrameTime > ANIMATION_SPEED) {
                currentFrame = (currentFrame + 1) % player_TOTAL_FRAMES;
                lastFrameTime = currentTime;
                // cout << "Current Frame: " << currentFrame << std::endl;
            }
        }
        else {
            currentFrame = 0;
        }
        if (player_POS_X > SCREEN_WIDTH - player_FRAME_WIDTH) {
            player_POS_X = SCREEN_WIDTH - player_FRAME_WIDTH;
        }
        
        player_POS_X += player_DIRECTION_X * player_SPEED * deltaTime;

        
        
        playerCenterPt.x = playerFrame.x + (playerFrame.w / 2); // Center x
        playerCenterPt.y = playerFrame.y + (playerFrame.h / 2); // Center y


        SDL_RenderClear(renderer);
        setImagePosition(bg_POS, 640, 360, bg_POS_X, bg_POS_Y);
        setImagePosition(playerFrame, player_FRAME_WIDTH * 2, player_FRAME_HEIGHT * 2, player_POS_X, player_POS_Y);
        setImagePosition(playerPortion, player_FRAME_WIDTH, player_FRAME_HEIGHT, player_FRAME_WIDTH * currentFrame, 0 );
        setImagePosition(grass_block_POS, polyT_WIDTH, polyT_HEIGHT, 0, getGroundBlockPosY(
        grass_block_POS, polyT_HEIGHT));
        setImagePosition(dirt_block_POS, polyT_WIDTH, polyT_HEIGHT, 0, getGroundBlockPosY(
            gem_block_POS , polyT_HEIGHT) + polyT_HEIGHT);
        setImagePosition(dripstone_DOWN_POS, 32, 32, dripstone_POS_X, dripstone_POS_Y);



        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); 


        SDL_RenderTexture(renderer, bgTexture_PRAIRIE, NULL, &bg_POS);
        SDL_RenderTexture(renderer, grassBlockTexture, NULL, &grass_block_POS);
        for (int i = 0; i < 10; i++) {
            grass_block_POS.x = polyT_WIDTH * i;
            dirt_block_POS.x = polyT_WIDTH * i;
            SDL_RenderTexture(renderer, grassBlockTexture, NULL, &grass_block_POS);
            SDL_RenderTexture(renderer, dirtBlockTexture, NULL, &dirt_block_POS);
        }

        SDL_RenderTexture(renderer, dripstone_DOWNTexture, NULL, &dripstone_DOWN_POS);


        SDL_RenderTextureRotated(renderer, playerT, &playerPortion, &playerFrame, 0, &playerCenterPt, 
            playerFacing);
        // the final product fr fr 
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(bgTexture_PRAIRIE);
    SDL_DestroyTexture(playerT);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}