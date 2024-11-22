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
#define MAX_LOADED_BULLETS 10 // Define maximum number of loaded bullets
#define BULLET_RELOAD_TIME 1000 // Define bullet reload time in milliseconds
#define MAX_METEOR_SPEED 5.0f //define the maximum meteor speed

void render_start_screen(SDL_Renderer *renderer, TTF_Font *font); // startscreen prototype
void render_bullets(SDL_Renderer *renderer, SDL_Texture *bullet_texture, int loaded_bullets); //bullet prototype

typedef struct {
    SDL_Rect rect; // Rectangle for bullet
    int active; // Bullet active status
    int loaded; // Bullet loaded status
} Bullet;

typedef struct {
    SDL_Rect rect; // Rectangle for meteor
    int active; // Meteor active status
    float speed_x; // Meteor speed
    float speed_y;
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

void reset_meteor(Meteor *meteor, int score) {
    meteor->rect.x = rand() % SCREEN_WIDTH; // Randomize meteor x position
    if (meteor->rect.x + meteor->rect.w > SCREEN_WIDTH) { // Ensure meteor does not go out of screen from right
        meteor->rect.x = SCREEN_WIDTH - meteor->rect.w;
    }
    meteor->rect.y = -rand() % SCREEN_HEIGHT; // Randomize meteor y position
    meteor->rect.w = 50 + rand() % 20; // Randomize meteor width
    meteor->rect.h = meteor->rect.w; // Set meteor height equal to width
    meteor->active = 1; // Activate meteor
    meteor->speed_y = METEOR_SPEED_START + rand() % 2; // Randomize meteor speed
    meteor->speed_y += (score / 50) * METEOR_SPEED_INCREMENT; // Increase speed based on score
    if(score>= 100){
        meteor->speed_x += ((rand() % 2)*2 -1) * (1 + (rand()%2));
    }
    else {
        meteor->speed_x = 0; // Meteor moves horizontally only after score >= 100
    }
    if (fabs(meteor->speed_x)> MAX_METEOR_SPEED){
        meteor->speed_x = (meteor->speed_x > 0 ? 1 : -1) * MAX_METEOR_SPEED; // Limit meteor speed
    }
}

void display_game_over(SDL_Renderer *renderer, TTF_Font *font, int score) {
    SDL_Color color = {255, 0, 0, 255}; // Set text color to red
    SDL_Color button_color = {0, 255, 0, 255}; // Green color for button
    SDL_Color shadow_color = {0, 0, 0, 255}; // Black color for shadow

    TTF_Font *large_font = TTF_OpenFont("./PressStart2P.ttf", 36); // Load larger font for "GAME OVER"
    TTF_Font *small_font = TTF_OpenFont("./PressStart2P.ttf", 24); // Load smaller font for score and button

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
    SDL_RenderClear(renderer); // Clear renderer

    // Render "GAME OVER" text with shadow
    SDL_Surface *surface = TTF_RenderText_Solid(large_font, "GAME OVER", shadow_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {SCREEN_WIDTH / 2 - surface->w / 2 + 2, SCREEN_HEIGHT / 2 - 50 + 2, surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);

    surface = TTF_RenderText_Solid(large_font, "GAME OVER", color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    dest.x -= 2;
    dest.y -= 2;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);

    // Render score text with shadow
    char score_text[20];
    sprintf(score_text, "Score: %d", score);
    surface = TTF_RenderText_Solid(small_font, score_text, shadow_color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    int surface_w = surface->w;
    int surface_h = surface->h;
    dest = (SDL_Rect){SCREEN_WIDTH / 2 - surface_w / 2 + 2, SCREEN_HEIGHT / 2 + 20 + 2, surface_w, surface_h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);

    surface = TTF_RenderText_Solid(small_font, score_text, color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    dest.x -= 2;
    dest.y -= 2;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);

    // Render "HOME" button with shadow
    surface = TTF_RenderText_Solid(small_font, "HOME", shadow_color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect button_rect = {SCREEN_WIDTH / 2 - surface->w / 2 + 2, SCREEN_HEIGHT / 2 + 100 + 2, surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &button_rect);
    SDL_DestroyTexture(texture);

    surface = TTF_RenderText_Solid(small_font, "HOME", button_color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    button_rect.x -= 2;
    button_rect.y -= 2;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &button_rect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);

    TTF_CloseFont(large_font);
    TTF_CloseFont(small_font);

    SDL_Event event;
    int waiting = 1;
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    waiting = 0;
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_Quit();
                    exit(0);
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= button_rect.x && x <= button_rect.x + button_rect.w && y >= button_rect.y && y <= button_rect.y + button_rect.h) {
                    render_start_screen(renderer, font);
                    waiting = 0;
                }
            }
        }
        SDL_Delay(100);
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

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *file) {
    SDL_Surface *surface = IMG_Load(file); // Load image
    if (!surface) {
        printf("Image loading failed: %s\n", IMG_GetError()); // Print error message
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface); // Create texture from surface
    SDL_FreeSurface(surface); // Free surface
    return texture;
}

void render_pause_icon(SDL_Renderer *renderer, SDL_Texture *pause_texture) {
    SDL_Rect pause_rect = {SCREEN_WIDTH / 2 - 15, 10, 30, 30}; // Increase pause icon size
    SDL_RenderCopy(renderer, pause_texture, NULL, &pause_rect); // Render pause icon
}

void render_bullets(SDL_Renderer *renderer, SDL_Texture *bullet_texture, int loaded_bullets) {
    for (int i = 0; i < MAX_LOADED_BULLETS; i++) {
        SDL_SetTextureAlphaMod(bullet_texture, i < loaded_bullets ? 255 : 128); // Set bullet opacity based on loaded status
        SDL_Rect bullet_rect = {SCREEN_WIDTH - 15 - i * 15, SCREEN_HEIGHT - 20, 10, 20}; // Position bullets in bottom right corner
        SDL_RenderCopy(renderer, bullet_texture, NULL, &bullet_rect); // Render bullet
    }
}

void render_start_screen(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color button_color = {0, 255, 0, 255}; // Green color for button
    SDL_Color title_color = {255, 0, 0, 255}; // red color for title
    SDL_Color subtitle_color = {150, 0, 0, 255}; // dark red color for subtitle

    SDL_Surface *title_surface = TTF_RenderText_Solid(font, "SPACE RIPPER", title_color);
    SDL_Texture *title_texture = SDL_CreateTextureFromSurface(renderer, title_surface);
    SDL_FreeSurface(title_surface);

    SDL_Rect title_rect = {SCREEN_WIDTH / 2 - title_surface->w / 2, SCREEN_HEIGHT / 4, title_surface->w, title_surface->h};

    TTF_Font *subtitle_font = TTF_OpenFont("./PressStart2P.ttf", 14); // Load smaller font for subtitle
    SDL_Surface *subtitle_surface = TTF_RenderText_Solid(subtitle_font, "FEEL THE FURY OF SPACE", subtitle_color);
    SDL_Texture *subtitle_texture = SDL_CreateTextureFromSurface(renderer, subtitle_surface);
    SDL_FreeSurface(subtitle_surface);

    SDL_Rect subtitle_rect = {SCREEN_WIDTH / 2 - subtitle_surface->w / 2, SCREEN_HEIGHT / 4 + 40, subtitle_surface->w, subtitle_surface->h};

    SDL_Surface *button_surface = TTF_RenderText_Solid(font, "START", button_color);
    SDL_Texture *button_texture = SDL_CreateTextureFromSurface(renderer, button_surface);
    SDL_FreeSurface(button_surface);

    SDL_Rect button_rect = {SCREEN_WIDTH / 2 - button_surface->w / 2, SCREEN_HEIGHT / 2, button_surface->w, button_surface->h};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, title_texture, NULL, &title_rect); // Render title
    SDL_RenderCopy(renderer, subtitle_texture, NULL, &subtitle_rect); // Render subtitle
    SDL_RenderCopy(renderer, button_texture, NULL, &button_rect); // Render button

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(title_texture);
    SDL_DestroyTexture(subtitle_texture);
    SDL_DestroyTexture(button_texture);
    TTF_CloseFont(subtitle_font); // Close subtitle font

    int waiting = 1;
    SDL_Event event;
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= button_rect.x && x <= button_rect.x + button_rect.w && y >= button_rect.y && y <= button_rect.y + button_rect.h) {
                    waiting = 0;
                }
            }
        }
        SDL_Delay(100);
    }
}

void render_hearts(SDL_Renderer *renderer, SDL_Texture *red_heart_texture, SDL_Texture *white_heart_texture, int lives) {
    for (int i = 0; i < INITIAL_LIFE; i++) {
        SDL_Texture *texture = i < lives ? red_heart_texture : white_heart_texture;
        SDL_Rect heart_rect = {SCREEN_WIDTH - 30 - i * 30, 10, 20, 20}; // Position hearts in the top right corner
        SDL_RenderCopy(renderer, texture, NULL, &heart_rect);
    }
}

void render_bullet(SDL_Renderer *renderer, SDL_Texture *bullet_texture, SDL_Rect *bullet_rect) {
    SDL_RenderCopy(renderer, bullet_texture, NULL, bullet_rect); // Render bullet using bullet texture
}

void render_active_bullets(SDL_Renderer *renderer, SDL_Texture *active_bullet_texture, Bullet bullets[], int bullet_count) {
    for (int i = 0; i < bullet_count; i++) {
        if (bullets[i].active) { // If bullet is active
            render_bullet(renderer, active_bullet_texture, &bullets[i].rect); // Render bullet using active bullet texture
        }
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

    TTF_Font *font = TTF_OpenFont("./PressStart2P.ttf", 20); // Load font
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

    SDL_Surface *spaceship_surface = SDL_LoadBMP("images/spaceship.bmp"); // Load spaceship image
    SDL_Surface *meteor_surface = IMG_Load("images/meteor(2).png"); // Load meteor image
    SDL_Texture *spaceship_texture = SDL_CreateTextureFromSurface(renderer, spaceship_surface); // Create spaceship texture
    SDL_Texture *meteor_texture = SDL_CreateTextureFromSurface(renderer, meteor_surface); // Create meteor texture

    SDL_FreeSurface(spaceship_surface); // Free spaceship surface
    SDL_FreeSurface(meteor_surface); // Free meteor surface

    SDL_Texture *pause_texture = load_texture(renderer, "images/pause-icon.png"); // Load pause icon texture
    if (!pause_texture) {
        // Handle error if pause icon texture fails to load
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *bullet_texture = load_texture(renderer, "images/bullet.png"); // Load bullet texture for bullets on the right
    if (!bullet_texture) {
        // Handle error if bullet texture fails to load
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *active_bullet_texture = load_texture(renderer, "images/bullet2.png"); // Load bullet texture for active bullets
    if (!active_bullet_texture) {
        // Handle error if active bullet texture fails to load
        SDL_DestroyTexture(bullet_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *red_heart_texture = load_texture(renderer, "images/red-heart.png"); // Load red heart texture
    if (!red_heart_texture) {
        // Handle error if red heart texture fails to load
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *white_heart_texture = load_texture(renderer, "images/white-heart.png"); // Load white heart texture
    if (!white_heart_texture) {
        // Handle error if white heart texture fails to load
        SDL_DestroyTexture(red_heart_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    render_start_screen(renderer, font); // Render start screen

    render_countdown(renderer, font); // Render countdown before starting the game

    Star stars[STAR_COUNT]; // Array of stars
    srand((unsigned int)time(NULL)); // Seed random number generator
    generate_stars(stars, STAR_COUNT); // Generate stars

    SDL_Rect spaceship = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 70, 50, 50}; // Move spaceship just above the bullets
    Bullet bullets[MAX_BULLETS]; // Array of bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0; // Deactivate bullet
    }
    Meteor meteors[MAX_METEORS]; // Array of meteors
    for (int i = 0; i < MAX_METEORS; i++) {
        meteors[i].active = 0; // Deactivate meteor
        reset_meteor(&meteors[i], 0); // Reset meteor
    }

    int running = 1; // Game running flag
    int paused = 0; // Game paused flag
    int score = 0; // Initialize score
    int lives = INITIAL_LIFE; // Initialize lives
    int loaded_bullets = MAX_LOADED_BULLETS; // Initialize loaded bullets
    Uint32 last_bullet_time = 0; // Initialize last bullet fire time
    Uint32 last_reload_time = SDL_GetTicks(); // Initialize last reload time

    while (running) {
        SDL_Event event; // Event variable
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { // Quit event
                running = 0; // Stop running
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE && !paused && loaded_bullets > 0 && SDL_GetTicks() - last_bullet_time > 200) { // Space key event with reduced delay
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].active) { // If bullet is not active
                            bullets[i].rect.x = spaceship.x + spaceship.w / 2 - bullets[i].rect.w / 2; // Set bullet x position
                            bullets[i].rect.y = spaceship.y; // Set bullet y position
                            bullets[i].rect.w = 10; // Set bullet width
                            bullets[i].rect.h = 10; // Set bullet height
                            bullets[i].active = 1; // Activate bullet
                            loaded_bullets--; // Decrease loaded bullets
                            last_bullet_time = SDL_GetTicks(); // Update last bullet fire time
                            break; // Break after firing one bullet
                        }
                    }
                } else if (event.key.keysym.sym == SDLK_ESCAPE) { // ESC key event
                    paused = !paused; // Toggle pause state
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                SDL_Rect pause_rect = {SCREEN_WIDTH / 2 - 15, 10, 30, 30}; // Pause icon rectangle
                if (x >= pause_rect.x && x <= pause_rect.x + pause_rect.w && y >= pause_rect.y && y <= pause_rect.y + pause_rect.h) {
                    paused = !paused; // Toggle pause state
                }
            }
        }

        if (!paused) {
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
                    meteors[i].rect.y += (int)meteors[i].speed_y; // Move meteor down
                    meteors[i].rect.x += (int)meteors[i].speed_x; // Move meteor horzontally

                     
                    if(meteors[i].rect.x < 0){// checking edge
                        meteors[i].rect.x = 0; // reset x position
                        meteors[i].speed_x = -meteors[i].speed_x; // change direction
                        meteors[i].speed_x = fabs(meteors[i].speed_x);
                    }
                    else if(meteors[i].rect.x > SCREEN_WIDTH - meteors[i].rect.w) {
                        meteors[i].rect.x = SCREEN_WIDTH - meteors[i].rect.w; // reset edge
                        meteors[i].speed_x = -fabs(meteors[i].speed_x);
                    }
            

                    if (meteors[i].rect.y > SCREEN_HEIGHT) { // If meteor is out of screen
                        reset_meteor(&meteors[i], score); // Reset meteor
                        // Do not increment score here
                    }

                    if (SDL_HasIntersection(&spaceship, &meteors[i].rect)) { // If spaceship collides with meteor
                        lives--; // Decrement lives
                        reset_meteor(&meteors[i], score); // Reset meteor
                        if (lives <= 0) { // If no lives left
                            display_game_over(renderer, font, score); // Display game over screen
                            score = 0; // Reset score
                            lives = INITIAL_LIFE; // Reset lives
                            i = 0; // Reset meteor index
                        }
                    }

                    for (int j = 0; j < MAX_BULLETS; j++) {
                        if (bullets[j].active && SDL_HasIntersection(&bullets[j].rect, &meteors[i].rect)) { // If bullet collides with meteor
                            reset_meteor(&meteors[i], score); // Reset meteor
                            bullets[j].active = 0; // Deactivate bullet
                            score++; // Increment score
                        }
                    }
                }
            }

            // Reload bullets
            if (SDL_GetTicks() - last_reload_time > BULLET_RELOAD_TIME && loaded_bullets < MAX_LOADED_BULLETS) {
                loaded_bullets++;
                last_reload_time = SDL_GetTicks();
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
        SDL_RenderClear(renderer); // Clear renderer

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set star color to white
        for (int i = 0; i < STAR_COUNT; i++) {
            SDL_RenderDrawPoint(renderer, stars[i].x, stars[i].y); // Render stars
        }

        SDL_RenderCopy(renderer, spaceship_texture, NULL, &spaceship); // Render spaceship
        render_active_bullets(renderer, active_bullet_texture, bullets, MAX_BULLETS); // Render active bullets

        for (int i = 0; i < MAX_METEORS; i++) {
            if (meteors[i].active) { // If meteor is active
                SDL_RenderCopy(renderer, meteor_texture, NULL, &meteors[i].rect); // Render meteor
            }
        }

        render_text(renderer, font, score, 10, 10); // Display score
        render_hearts(renderer, red_heart_texture, white_heart_texture, lives); // Render hearts
        render_pause_icon(renderer, pause_texture); // Render pause icon
        render_bullets(renderer, bullet_texture, loaded_bullets); // Render bullets in bottom right corner

        SDL_RenderPresent(renderer); // Present renderer
        SDL_Delay(1000 / 60); // Delay for 60 FPS
    }

    SDL_DestroyTexture(active_bullet_texture); // Destroy active bullet texture
    SDL_DestroyTexture(white_heart_texture); // Destroy white heart texture
    SDL_DestroyTexture(red_heart_texture); // Destroy red heart texture
    SDL_DestroyTexture(bullet_texture); // Destroy bullet texture
    SDL_DestroyTexture(pause_texture); // Destroy pause icon texture
    SDL_DestroyTexture(spaceship_texture); // Destroy spaceship texture
    SDL_DestroyTexture(meteor_texture); // Destroy meteor texture
    TTF_CloseFont(font); // Close font
    TTF_Quit(); // Quit SDL_ttf
    SDL_DestroyRenderer(renderer); // Destroy renderer
    SDL_DestroyWindow(window); // Destroy window
    SDL_Quit(); // Quit SDL

    return 0; // Return success code
}
