#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WIDTH 900
#define HEIGHT 600
#define FPS 60
#define SCALE_FACTOR 400

// Higher the time steps, higher the simulation accuracy. See: https://en.wikipedia.org/wiki/Euler_method
#define TIME_STEPS 10

#define L_MASS 1
#define R_MASS 1000000  // Must be a power of 100 in order to the collisions number be ~ pi

#define HEX(c)                 \
  ((c >> (8 * 3)) & 0xFF),     \
      ((c >> (8 * 2)) & 0xFF), \
      ((c >> (8 * 1)) & 0xFF), \
      ((c >> (8 * 0)) & 0xFF)

typedef struct {
  float dx;
  float mass;
  SDL_FRect rect;
} Block;

static unsigned int collisions = 0;

void update(float dt, Block *block_l, Block *block_r) {
  block_l->rect.x += block_l->dx * dt;
  block_r->rect.x += block_r->dx * dt;

  if (block_l->rect.x + block_l->rect.w >= block_r->rect.x) {
    float old_l_dx = block_l->dx;
    float old_r_dx = block_r->dx;
    block_l->dx = (block_l->mass - block_r->mass) / (block_l->mass + block_r->mass) * old_l_dx + (2 * block_r->mass * old_r_dx) / (block_l->mass + block_r->mass);
    block_r->dx = (block_r->mass - block_l->mass) / (block_l->mass + block_r->mass) * old_r_dx + (2 * block_l->mass * old_l_dx) / (block_l->mass + block_r->mass);
    collisions++;
    printf("INFO: Block intersection detected. New left speed: %.2f. New right speed: %.2f\n", block_l->dx, block_r->dx);
    printf("INFO: Current collisions number: %u\n", collisions);
  }

  if (block_l->rect.x <= 30) {
    block_l->dx *= -1;
    collisions++;
    printf("INFO: Wall intersection detected. New left speed: %.2f. \n", block_l->dx);
    printf("INFO: Current collisions number: %u\n", collisions);
  }
}

void render(SDL_Renderer *renderer, Block *block_l, Block *block_r) {
  SDL_SetRenderDrawColor(renderer, HEX(0x181818FF));
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, HEX(0xFFFFFFFF));
  SDL_RenderDrawLine(renderer, 30, HEIGHT - 30, WIDTH, HEIGHT - 30);
  SDL_RenderDrawLine(renderer, 30, 40, 30, HEIGHT - 30);

  float old_l_x = block_l->rect.x;
  float old_r_x = block_r->rect.x;

  if (block_r->rect.x <= 30 + block_l->rect.w) {
    block_r->rect.x = 30 + block_l->rect.w;
    block_l->rect.x = 30;
  } else if (block_l->rect.x < 30) {
    block_l->rect.x = 30;
  }

  SDL_RenderFillRectF(renderer, &block_l->rect);
  SDL_RenderFillRectF(renderer, &block_r->rect);

  SDL_RenderPresent(renderer);
  block_l->rect.x = old_l_x;
  block_r->rect.x = old_r_x;
}

int main(void) {
  static_assert(L_MASS <= R_MASS, "Left block mass must be less or equal than right block mass.");
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
  int block_l_sz = fmin(200, sqrt(L_MASS * SCALE_FACTOR) + 25);
  int block_r_sz = fmin(400, sqrt(R_MASS * SCALE_FACTOR) + 25);

  Block block_l = {
      .dx = 0,
      .mass = L_MASS,
      .rect = {
          .x = 250,
          .y = HEIGHT - 30 - block_l_sz,
          .w = block_l_sz,
          .h = block_l_sz,
      },
  };

  Block block_r = {
      .dx = -2.0f / TIME_STEPS,
      .mass = R_MASS,
      .rect = {
          .x = WIDTH - 300,
          .y = HEIGHT - 30 - block_r_sz,
          .w = block_r_sz,
          .h = block_r_sz,
      },
  };

  while (!quit) {
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT: {
          quit = true;
        } break;
      }
    }

    for (int i = 0; i < TIME_STEPS; ++i) {
      update(1, &block_l, &block_r);
    }
    render(renderer, &block_l, &block_r);

    usleep(1000 * 1000 / FPS);
  }

  SDL_Quit();

  return 0;
}