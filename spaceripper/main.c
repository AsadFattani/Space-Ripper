#include <SDL2/SDL.h> // Include SDL library
#include <SDL2/SDL_ttf.h> // Include SDL_ttf library for text rendering
#include <SDL2/SDL_image.h> // Include SDL_image library for image loading
#include <stdio.h> // Include standard I/O library
#include <stdlib.h> // Include standard library
#include <time.h> // Include time library for random seed

#define SCREEN_WIDTH 400 // Define screen width
#define SCREEN_HEIGHT 625 // Define screen height
#define SHIP_SPEED 5 // Define ship speed
#define BULLET_SPEED 10 // Define bullet speed
#define METEOR_SPEED_START 3 // Define initial meteor speed
#define METEOR_SPEED_INCREMENT 0.2 // Define meteor speed increment
#define STAR_COUNT 100 // Define number of stars
#define INITIAL_LIFE 5 // Define initial lives
#define MAX_METEORS 6 // Define maximum number of meteors
#define MAX_BULLETS 10 // Define maximum number of bullets

typedef struct {
    SDL_Rect rect; // Rectangle for bullet
    int active; // Bullet active status
} Bullet;

typedef struct {
    SDL_Rect rect; // Rectangle for meteor
    int active; // Meteor active status
    float speed; // Meteor speed
} Meteor;

typedef struct {
    int x, y; // Coordinates for star
} Star;

void generate_stars(Star stars[], int count) {
    for (int i = 0; i < count; i++) {
        stars[i].x = rand() % SCREEN_WIDTH; // Randomize star x position
        stars[i].y = rand() % SCREEN_HEIGHT; // Randomize star y position
    }
}

void render_text(SDL_Renderer *renderer, TTF_Font *font, int value, int x, int y) {
    SDL_Color color = {255, 255, 255, 255}; // Set text color to white
    char value_str[10]; // Buffer for text
    sprintf(value_str, "%d", value); // Convert value to string

    SDL_Surface *surface = TTF_RenderText_Solid(font, value_str, color); // Create surface from text
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface); // Create texture from surface
    SDL_FreeSurface(surface); // Free surface

    SDL_Rect dest = {x, y, surface->w, surface->h}; // Set destination rectangle
    SDL_RenderCopy(renderer, texture, NULL, &dest); // Render text
    SDL_DestroyTexture(texture); // Destroy texture
}

void reset_meteor(Meteor *meteor) {
    meteor->rect.x = rand() % SCREEN_WIDTH; // Randomize meteor x position
    meteor->rect.y = -rand() % SCREEN_HEIGHT; // Randomize meteor y position
    meteor->rect.w = 50 + rand() % 20; // Randomize meteor width
    meteor->rect.h = meteor->rect.w; // Set meteor height equal to width
    meteor->active = 1; // Activate meteor
    meteor->speed = METEOR_SPEED_START + rand() % 2; // Randomize meteor speed
}

void display_game_over(SDL_Renderer *renderer, TTF_Font *font, int score) {
    SDL_Color color = {255, 0, 0, 255}; // Set text color to red
    SDL_Surface *surface = TTF_RenderText_Solid(font, "GAME OVER", color); // Create surface for "GAME OVER"
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface); // Create texture from surface
    SDL_FreeSurface(surface); // Free surface

    SDL_Rect dest = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 50}; // Set destination rectangle
    
    SDL_SetRenderDrawColor(renderer, 28, 27, 27, 255); // Set background color to grey
    SDL_RenderClear(renderer); // Clear renderer
    SDL_RenderCopy(renderer, texture, NULL, &dest); // Render "GAME OVER" text
    SDL_DestroyTexture(texture); // Destroy texture

    char score_text[20]; // Buffer for score text
    sprintf(score_text, "Score: %d", score); // Convert score to string
    surface = TTF_RenderText_Solid(font, score_text, color); // Create surface for score
    texture = SDL_CreateTextureFromSurface(renderer, surface); // Create texture from surface
    SDL_FreeSurface(surface); // Free surface

    dest.y += 60; // Position below "GAME OVER"
    dest.w = 100; // Set width
    dest.h = 25; // Set height
    SDL_RenderCopy(renderer, texture, NULL, &dest); // Render score text
    SDL_RenderPresent(renderer); // Present renderer
    SDL_DestroyTexture(texture); // Destroy texture

    SDL_Event event; // Event variable
    int waiting = 1; // Waiting flag
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { // Quit event
                SDL_Quit(); // Quit SDL
                exit(0); // Exit program
            } else if (event.type == SDL_KEYDOWN) { // Key down event
                if (event.key.keysym.sym == SDLK_RETURN) { // Enter key
                    waiting = 0; // Stop waiting
                } else if (event.key.keysym.sym == SDLK_ESCAPE) { // Escape key
                    SDL_Quit(); // Quit SDL
                    exit(0); // Exit program
                }
            }
        }
        SDL_Delay(100); // Delay for 100 ms
    }
}

void render_countdown(SDL_Renderer *renderer, TTF_Font *font) {
    for (int i = 3; i > 0; i--) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
        SDL_RenderClear(renderer); // Clear renderer
        render_text(renderer, font, i, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT / 2 - 15); // Render countdown number
        SDL_RenderPresent(renderer); // Present renderer
        SDL_Delay(1000); // Delay for 1 second
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { // Initialize SDL
        printf("SDL initialization failed: %s\n", SDL_GetError()); // Print error message
        return 1; // Return error code
    }

    if (TTF_Init() == -1) { // Initialize SDL_ttf
        printf("TTF initialization failed: %s\n", TTF_GetError()); // Print error message
        SDL_Quit(); // Quit SDL
        return 1; // Return error code
    }

    TTF_Font *font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24); // Load font
    if (!font) { // Check if font loaded
        printf("Font loading failed: %s\n", TTF_GetError()); // Print error message
        TTF_Quit(); // Quit SDL_ttf
        SDL_Quit(); // Quit SDL
        return 1; // Return error code
    }

    SDL_Window *window = SDL_CreateWindow("Spaceship Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); // Create window
    if (!window) { // Check if window created
        printf("Window creation failed: %s\n", SDL_GetError()); // Print error message
        TTF_CloseFont(font); // Close font
        TTF_Quit(); // Quit SDL_ttf
        SDL_Quit(); // Quit SDL
        return 1; // Return error code
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Create renderer
    if (!renderer) { // Check if renderer created
        printf("Renderer creation failed: %s\n", SDL_GetError()); // Print error message
        SDL_DestroyWindow(window); // Destroy window
        TTF_CloseFont(font); // Close font
        TTF_Quit(); // Quit SDL_ttf
        SDL_Quit(); // Quit SDL
        return 1; // Return error code
    }

    SDL_Surface *spaceship_surface = SDL_LoadBMP("spaceship.bmp"); // Load spaceship image
    SDL_Surface *meteor_surface = IMG_Load("meteor(2).png"); // Load meteor image
    SDL_Texture *spaceship_texture = SDL_CreateTextureFromSurface(renderer, spaceship_surface); // Create spaceship texture
    SDL_Texture *meteor_texture = SDL_CreateTextureFromSurface(renderer, meteor_surface); // Create meteor texture

    SDL_FreeSurface(spaceship_surface); // Free spaceship surface
    SDL_FreeSurface(meteor_surface); // Free meteor surface

    render_countdown(renderer, font); // Render countdown before starting the game

    Star stars[STAR_COUNT]; // Array of stars
    srand((unsigned int)time(NULL)); // Seed random number generator
    generate_stars(stars, STAR_COUNT); // Generate stars

    SDL_Rect spaceship = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50}; // Initialize spaceship rectangle
    Bullet bullets[MAX_BULLETS]; // Array of bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0; // Deactivate bullet
    }
    Meteor meteors[MAX_METEORS]; // Array of meteors
    for (int i = 0; i < MAX_METEORS; i++) {
        meteors[i].active = 0; // Deactivate meteor
        reset_meteor(&meteors[i]); // Reset meteor
    }

    int running = 1; // Game running flag
    int score = 0; // Initialize score
    int lives = INITIAL_LIFE; // Initialize lives

    while (running) {
        SDL_Event event; // Event variable
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { // Quit event
                running = 0; // Stop running
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) { // Space key event
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) { // If bullet is not active
                        bullets[i].rect.x = spaceship.x + spaceship.w / 2 - bullets[i].rect.w / 2; // Set bullet x position
                        bullets[i].rect.y = spaceship.y; // Set bullet y position
                        bullets[i].rect.w = 10; // Set bullet width
                        bullets[i].rect.h = 10; // Set bullet height
                        bullets[i].active = 1; // Activate bullet
                        break; // Break after firing one bullet
                    }
                }
            }
        }

        const Uint8 *keystates = SDL_GetKeyboardState(NULL); // Get keyboard state
        if (keystates[SDL_SCANCODE_LEFT] && spaceship.x > 0) { // Left key event
            spaceship.x -= SHIP_SPEED; // Move spaceship left
        }
        if (keystates[SDL_SCANCODE_RIGHT] && spaceship.x + spaceship.w < SCREEN_WIDTH) { // Right key event
            spaceship.x += SHIP_SPEED; // Move spaceship right
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) { // If bullet is active
                bullets[i].rect.y -= BULLET_SPEED; // Move bullet up
                if (bullets[i].rect.y < 0) { // If bullet is out of screen
                    bullets[i].active = 0; // Deactivate bullet
                }
            }
        }

        for (int i = 0; i < MAX_METEORS; i++) {
            if (meteors[i].active) { // If meteor is active
                meteors[i].rect.y += (int)meteors[i].speed; // Move meteor down
                if (meteors[i].rect.y > SCREEN_HEIGHT) { // If meteor is out of screen
                    reset_meteor(&meteors[i]); // Reset meteor
                    score++; // Increment score
                }

                if (SDL_HasIntersection(&spaceship, &meteors[i].rect)) { // If spaceship collides with meteor
                    lives--; // Decrement lives
                    reset_meteor(&meteors[i]); // Reset meteor
                    if (lives <= 0) { // If no lives left
                        display_game_over(renderer, font, score); // Display game over screen
                        score = 0; // Reset score
                        lives = INITIAL_LIFE; // Reset lives
                        i = 0; // Reset meteor index
                    }
                }

                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (bullets[j].active && SDL_HasIntersection(&bullets[j].rect, &meteors[i].rect)) { // If bullet collides with meteor
                        reset_meteor(&meteors[i]); // Reset meteor
                        bullets[j].active = 0; // Deactivate bullet
                        score++; // Increment score
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
        SDL_RenderClear(renderer); // Clear renderer

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set star color to white
        for (int i = 0; i < STAR_COUNT; i++) {
            SDL_RenderDrawPoint(renderer, stars[i].x, stars[i].y); // Render stars
        }

        SDL_RenderCopy(renderer, spaceship_texture, NULL, &spaceship); // Render spaceship
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Set bullet color to red
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) { // If bullet is active
                SDL_RenderFillRect(renderer, &bullets[i].rect); // Render bullet
            }
        }

        for (int i = 0; i < MAX_METEORS; i++) {
            if (meteors[i].active) { // If meteor is active
                SDL_RenderCopy(renderer, meteor_texture, NULL, &meteors[i].rect); // Render meteor
            }
        }

        render_text(renderer, font, score, 10, 10); // Display score
        render_text(renderer, font, lives, SCREEN_WIDTH - 30, 10); // Display lives on the far right corner

        SDL_RenderPresent(renderer); // Present renderer
        SDL_Delay(1000 / 60); // Delay for 60 FPS
    }

    SDL_DestroyTexture(spaceship_texture); // Destroy spaceship texture
    SDL_DestroyTexture(meteor_texture); // Destroy meteor texture
    TTF_CloseFont(font); // Close font
    TTF_Quit(); // Quit SDL_ttf
    SDL_DestroyRenderer(renderer); // Destroy renderer
    SDL_DestroyWindow(window); // Destroy window
    SDL_Quit(); // Quit SDL

    return 0; // Return success code
}
