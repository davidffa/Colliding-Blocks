#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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

// Higher the time steps, higher the simulation accuracy.
#define TIME_STEPS 1000

#define PI_DIGITS 5
#define L_MASS 1
#define R_MASS (powf(100, PI_DIGITS))

#define CLACK_SOUND "./assets/clack.wav"
#define FONT_PATH "./assets/font.ttf"

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

static Uint8 *wav_buf;
static Uint32 wav_len;
static SDL_AudioDeviceID device_id;

static unsigned int collisions = 0;

static char text[128];

void play_clack() {
  if (SDL_GetQueuedAudioSize(device_id) == 0)
    SDL_QueueAudio(device_id, wav_buf, wav_len);
  SDL_PauseAudioDevice(device_id, 0);
}

void update(float dt, Block *block_l, Block *block_r) {
  block_l->rect.x += block_l->dx * dt;
  block_r->rect.x += block_r->dx * dt;

  if (block_l->rect.x + block_l->rect.w >= block_r->rect.x) {
    float old_l_dx = block_l->dx;
    float old_r_dx = block_r->dx;
    block_l->dx = (block_l->mass - block_r->mass) / (block_l->mass + block_r->mass) * old_l_dx + (2 * block_r->mass * old_r_dx) / (block_l->mass + block_r->mass);
    block_r->dx = (block_r->mass - block_l->mass) / (block_l->mass + block_r->mass) * old_r_dx + (2 * block_l->mass * old_l_dx) / (block_l->mass + block_r->mass);
    collisions++;
    play_clack();
    printf("INFO: Block intersection detected. New left speed: %.2f. New right speed: %.2f\n", block_l->dx, block_r->dx);
    printf("INFO: Current collisions number: %u\n", collisions);
  }

  if (block_l->rect.x <= 30) {
    block_l->dx *= -1;
    collisions++;
    play_clack();
    printf("INFO: Wall intersection detected. New left speed: %.2f. \n", block_l->dx);
    printf("INFO: Current collisions number: %u\n", collisions);
  }
}

void render(SDL_Renderer *renderer, Block *block_l, Block *block_r, TTF_Font *font) {
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

  sprintf(text, "Collisions: %u", collisions);

  SDL_Color white = {255, 255, 255, 255};
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, white);
  SDL_Texture *message = SDL_CreateTextureFromSurface(renderer, surface);

  int text_w, text_h;
  SDL_QueryTexture(message, NULL, NULL, &text_w, &text_h);

  SDL_Rect msg_rect = {
      .x = WIDTH - text_w - 30,
      .y = 30,
      .w = text_w,
      .h = text_h,
  };

  SDL_RenderCopy(renderer, message, NULL, &msg_rect);

  SDL_RenderPresent(renderer);
  block_l->rect.x = old_l_x;
  block_r->rect.x = old_r_x;

  SDL_FreeSurface(surface);
  SDL_DestroyTexture(message);
}

int main(void) {
  assert(L_MASS <= R_MASS && "Left block mass must be less or equal than right block mass.");
  int sdl = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

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

  TTF_Init();
  TTF_Font *font = TTF_OpenFont(FONT_PATH, 24);

  if (font == NULL) {
    fprintf(stderr, "ERROR: Failed to load font: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_AudioSpec wav_spec;

  if (SDL_LoadWAV(CLACK_SOUND, &wav_spec, &wav_buf, &wav_len) == NULL) {
    fprintf(stderr, "ERROR: Failed to load clack.wav: %s\n", SDL_GetError());
    exit(1);
  }

  device_id = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);

  bool quit = false;
  int block_l_sz = fmin(200, sqrt(L_MASS * SCALE_FACTOR) + 25);
  int block_r_sz = fmin(400, sqrt(R_MASS * SCALE_FACTOR / 2) + 25);

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
    render(renderer, &block_l, &block_r, font);

    usleep(1000 * 1000 / FPS);
  }

  SDL_CloseAudioDevice(device_id);
  SDL_FreeWAV(wav_buf);
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();

  return 0;
}