#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 360
#define SCREEN_HEIGHT 800
#define SHIP_SPEED 5
#define BULLET_SPEED 10

typedef struct {
    SDL_Rect rect;
    SDL_Surface* surface;
} Spaceship;

typedef struct {
    SDL_Rect rect;
    int active;
} Bullet;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Spaceship Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* spaceship_surface = SDL_LoadBMP("spaceship (1).bmp");
    if (!spaceship_surface) {
        printf("Spaceship image loading failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Spaceship spaceship;
    spaceship.rect.x = SCREEN_WIDTH / 2;
    spaceship.rect.y = SCREEN_HEIGHT - 60;
    spaceship.rect.w = 50; 
    spaceship.rect.h = 50; 
    spaceship.surface = spaceship_surface;

    Bullet bullet;
    bullet.rect.x = 0;
    bullet.rect.y = 0;
    bullet.rect.w = 10;
    bullet.rect.h = 10;
    bullet.active = 0;

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    // Shoot bullet
                    if (!bullet.active) {
                        bullet.rect.x = spaceship.rect.x + spaceship.rect.w / 2;
                        bullet.rect.y = spaceship.rect.y;
                        bullet.active = 1;
                    }
                }
            }
        }

        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_LEFT] && spaceship.rect.x > 0) {
            spaceship.rect.x -= SHIP_SPEED;
        }
        if (keystates[SDL_SCANCODE_RIGHT] && spaceship.rect.x + spaceship.rect.w < SCREEN_WIDTH) {
            spaceship.rect.x += SHIP_SPEED;
        }
        if (keystates[SDL_SCANCODE_UP] && spaceship.rect.y > 0) {
            spaceship.rect.y -= SHIP_SPEED;
        }
        if (keystates[SDL_SCANCODE_DOWN] && spaceship.rect.y + spaceship.rect.h < SCREEN_HEIGHT) {
            spaceship.rect.y += SHIP_SPEED;
        }

        if (bullet.active) {
            bullet.rect.y -= BULLET_SPEED;
            if (bullet.rect.y < 0) {
                bullet.active = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); 
        SDL_RenderClear(renderer);

        SDL_Texture* spaceship_texture = SDL_CreateTextureFromSurface(renderer, spaceship.surface);
        SDL_RenderCopy(renderer, spaceship_texture, NULL, &spaceship.rect);
        SDL_DestroyTexture(spaceship_texture);

        if (bullet.active) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &bullet.rect);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / 60);
    }

    SDL_FreeSurface(spaceship.surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
