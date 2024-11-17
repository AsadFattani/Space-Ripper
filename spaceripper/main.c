#include <SDL2/SDL.h>
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

void render_text(SDL_Renderer *renderer, int value, SDL_Texture *numbers[], int x, int y) {
    char value_str[10];
    sprintf(value_str, "%d", value);

    for (int i = 0; value_str[i] != '\0'; i++) {
        SDL_Rect dest = {x, y, 20, 30};
        SDL_RenderCopy(renderer, numbers[value_str[i] - '0'], NULL, &dest);
        x += 25;
    }
}

void reset_meteor(Meteor *meteor) {
    meteor->rect.x = rand() % SCREEN_WIDTH;
    meteor->rect.y = -rand() % SCREEN_HEIGHT;
    meteor->rect.w = 50 + rand() % 20; // Randomize size
    meteor->rect.h = meteor->rect.w;
    meteor->active = 1;
    meteor->speed = METEOR_SPEED_START + rand() % 2;
}

void display_game_over(SDL_Renderer *renderer, SDL_Texture *end_screen) {
    SDL_RenderCopy(renderer, end_screen, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Event event;
    int waiting = 1;
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
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

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Spaceship Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *spaceship_surface = SDL_LoadBMP("spaceship (1).bmp");
    SDL_Surface *meteor_surface = SDL_LoadBMP("meteor.bmp");
    SDL_Surface *end_screen_surface = SDL_LoadBMP("endscreen.bmp");
    SDL_Surface *number_surfaces[10];
    SDL_Texture *number_textures[10];
    SDL_Texture *spaceship_texture = SDL_CreateTextureFromSurface(renderer, spaceship_surface);
    SDL_Texture *meteor_texture = SDL_CreateTextureFromSurface(renderer, meteor_surface);
    SDL_Texture *end_screen_texture = SDL_CreateTextureFromSurface(renderer, end_screen_surface);

    for (int i = 0; i < 10; i++) {
        char filename[20];
        sprintf(filename, "number_%d.bmp", i);
        number_surfaces[i] = SDL_LoadBMP(filename);
        number_textures[i] = SDL_CreateTextureFromSurface(renderer, number_surfaces[i]);
        SDL_FreeSurface(number_surfaces[i]);
    }

    SDL_FreeSurface(spaceship_surface);
    SDL_FreeSurface(meteor_surface);
    SDL_FreeSurface(end_screen_surface);

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
                        display_game_over(renderer, end_screen_texture);
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

        render_text(renderer, score, number_textures, 10, 10); // Display score
        render_text(renderer, lives, number_textures, 300, 10); // Display lives

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyTexture(spaceship_texture);
    SDL_DestroyTexture(meteor_texture);
    SDL_DestroyTexture(end_screen_texture);
    for (int i = 0; i < 10; i++) {
        SDL_DestroyTexture(number_textures[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
