#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define WIDTH 900
#define HEIGHT 600

#define HEX(c)                 \
  ((c >> (8 * 3)) & 0xFF),     \
      ((c >> (8 * 2)) & 0xFF), \
      ((c >> (8 * 1)) & 0xFF), \
      ((c >> (8 * 0)) & 0xFF)

void update() {
}

void render(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, HEX(0x181818FF));
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
}

int main(void) {
  int sdl = SDL_Init(SDL_INIT_VIDEO);

  if (sdl != 0) {
    fprintf(stderr, "ERROR: SDL init failed: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Window *window = SDL_CreateWindow("Colliding Blocks", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  if (window == NULL) {
    fprintf(stderr, "ERROR: Failed to create the window: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL) {
    fprintf(stderr, "ERROR: Failed to create the renderer: %s\n", SDL_GetError());
    exit(1);
  }

  bool quit = false;

  while (!quit) {
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT: {
          quit = true;
        } break;
      }
    }

    update();
    render(renderer);
  }

  SDL_Quit();

  return 0;
}