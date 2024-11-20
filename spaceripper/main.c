#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 625
#define SHIP_SPEED 5
#define BULLET_SPEED 10
#define METEOR_SPEED_START 3
#define METEOR_SPEED_INCREMENT 0.2
#define STAR_COUNT 100
#define INITIAL_LIFE 5
#define MAX_METEORS 6

typedef struct {
    SDL_Rect rect;
    int active;
} Bullet;

typedef struct {
    SDL_Rect rect;
    int active;
    float speed;
} Meteor;

typedef struct {
    int x, y;
} Star;

void generate_stars(Star stars[], int count) {
    for (int i = 0; i < count; i++) {
        stars[i].x = rand() % SCREEN_WIDTH;
        stars[i].y = rand() % SCREEN_HEIGHT;
    }
}

void render_text(SDL_Renderer *renderer, TTF_Font *font, int value, int x, int y) {
    SDL_Color color = {255, 255, 255, 255};
    char value_str[10];
    sprintf(value_str, "%d", value);

    SDL_Surface *surface = TTF_RenderText_Solid(font, value_str, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

void reset_meteor(Meteor *meteor) {
    meteor->rect.x = rand() % SCREEN_WIDTH;
    meteor->rect.y = -rand() % SCREEN_HEIGHT;
    meteor->rect.w = 50 + rand() % 20; // Randomize size
    meteor->rect.h = meteor->rect.w;
    meteor->active = 1;
    meteor->speed = METEOR_SPEED_START + rand() % 2;
}

void display_game_over(SDL_Renderer *renderer, TTF_Font *font, int score) {
    SDL_Color color = {255, 0, 0, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, "GAME OVER", color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect dest = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 50};
    
    SDL_SetRenderDrawColor(renderer, 28, 27, 27, 255); // Set background to grey
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);

    char score_text[20];
    sprintf(score_text, "Score: %d", score);
    surface = TTF_RenderText_Solid(font, score_text, color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    dest.y += 60; // Position below "GAME OVER"
    dest.w = 100;
    dest.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);

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
            }
        }
        SDL_Delay(100);
    }
}

void render_countdown(SDL_Renderer *renderer, TTF_Font *font) {
    for (int i = 3; i > 0; i--) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render_text(renderer, font, i, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT / 2 - 15);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000);
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("TTF initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24); // Ensure the correct path to the font file
    if (!font) {
        printf("Font loading failed: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Spaceship Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface *spaceship_surface = SDL_LoadBMP("spaceship (1).bmp");
    SDL_Surface *meteor_surface = SDL_LoadBMP("meteor.bmp");
    SDL_Surface *end_screen_surface = SDL_LoadBMP("endscreen.bmp");
    SDL_Texture *spaceship_texture = SDL_CreateTextureFromSurface(renderer, spaceship_surface);
    SDL_Texture *meteor_texture = SDL_CreateTextureFromSurface(renderer, meteor_surface);
    SDL_Texture *end_screen_texture = SDL_CreateTextureFromSurface(renderer, end_screen_surface);

    SDL_FreeSurface(spaceship_surface);
    SDL_FreeSurface(meteor_surface);
    SDL_FreeSurface(end_screen_surface);

    render_countdown(renderer, font); // Add countdown before starting the game

    Star stars[STAR_COUNT];
    srand((unsigned int)time(NULL));
    generate_stars(stars, STAR_COUNT);

    SDL_Rect spaceship = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50};
    Bullet bullet = {{0, 0, 10, 10}, 0};
    Meteor meteors[MAX_METEORS];
    for (int i = 0; i < MAX_METEORS; i++) {
        meteors[i].active = 0;
        reset_meteor(&meteors[i]);
    }

    int running = 1;
    int score = 0;
    int lives = INITIAL_LIFE;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (!bullet.active) {
                    bullet.rect.x = spaceship.x + spaceship.w / 2 - bullet.rect.w / 2;
                    bullet.rect.y = spaceship.y;
                    bullet.active = 1;
                }
            }
        }

        const Uint8 *keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_LEFT] && spaceship.x > 0) {
            spaceship.x -= SHIP_SPEED;
        }
        if (keystates[SDL_SCANCODE_RIGHT] && spaceship.x + spaceship.w < SCREEN_WIDTH) {
            spaceship.x += SHIP_SPEED;
        }

        if (bullet.active) {
            bullet.rect.y -= BULLET_SPEED;
            if (bullet.rect.y < 0) {
                bullet.active = 0;
            }
        }

        for (int i = 0; i < MAX_METEORS; i++) {
            if (meteors[i].active) {
                meteors[i].rect.y += (int)meteors[i].speed;
                if (meteors[i].rect.y > SCREEN_HEIGHT) {
                    reset_meteor(&meteors[i]);
                    score++;
                }

                if (SDL_HasIntersection(&spaceship, &meteors[i].rect)) {
                    lives--;
                    reset_meteor(&meteors[i]);
                    if (lives <= 0) {
                        display_game_over(renderer, font, score);
                        score = 0;
                        lives = INITIAL_LIFE;
                        i = 0;
                    }
                }

                if (bullet.active && SDL_HasIntersection(&bullet.rect, &meteors[i].rect)) {
                    reset_meteor(&meteors[i]);
                    bullet.active = 0;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < STAR_COUNT; i++) {
            SDL_RenderDrawPoint(renderer, stars[i].x, stars[i].y);
        }

        SDL_RenderCopy(renderer, spaceship_texture, NULL, &spaceship);
        if (bullet.active) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &bullet.rect);
        }

        for (int i = 0; i < MAX_METEORS; i++) {
            if (meteors[i].active) {
                SDL_RenderCopy(renderer, meteor_texture, NULL, &meteors[i].rect);
            }
        }

        render_text(renderer, font, score, 10, 10); // Display score
        render_text(renderer, font, lives, SCREEN_WIDTH - 30, 10); // Display lives on the far right corner

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyTexture(spaceship_texture);
    SDL_DestroyTexture(meteor_texture);
    SDL_DestroyTexture(end_screen_texture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
