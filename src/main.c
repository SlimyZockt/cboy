#include "memory.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <cJSON.h>
#include <emulation.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define CLOCK_SPEED 4194304
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#define WRAM 8192
#define VRAM 8192
#define SCREEN_X 160
#define SCREEN_Y 160

uint8_t read_file(const char *path, struct File *f) {
  enum ErrorCodes { OK, FOPEN, FSEEK, CALLOC, FREED, FCLOSE };

  FILE *file = fopen(path, "r");
  if (file == NULL) {
    printf("File not found.");
    return FOPEN;
  }
  uint32_t err = fseek(file, 0, SEEK_END);
  if (err != 0) {
    return FSEEK;
  }
  uint32_t file_size = ftell(file);
  err = fseek(file, 0, SEEK_SET);
  if (err != 0) {
    return FSEEK;
  }
  void *data = calloc(1, file_size);
  if (data == NULL) {
    return CALLOC;
  }

  uint32_t size = fread(data, 1, file_size, file);
  if (size == 0) {
    return FREED;
  }
  err = fclose(file);
  if (err != 0) {
    return FCLOSE;
  }
  f->size = file_size;
  f->data = data;
  return OK;
}

int run_sdl(void) {
  SDL_Window *window = NULL;
  SDL_Renderer *renerer;

  int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  if (res < 0) {
    SDL_Log("SLD_INIT error: %s", SDL_GetError());
    return -1;
  }

  window = SDL_CreateWindow("cboy", 592, 390, 0);
  if (window == NULL) {
    SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
    return -2;
  }

  renerer = SDL_CreateRenderer(window, NULL);
  if (renerer == NULL) {
    SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
    return -3;
  }

  SDL_Log("SDL3 init");

  SDL_Event event;
  bool quit = 0;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        SDL_Log("SDL3 event quit");
        quit = true;
        break;
      default:
        break;
      }

      SDL_Delay(1);
      /*cpu.cycleCount += 1;*/

      /*SDL_SetRenderDrawColor(renerer, 0, 0, 0, 0xFF);*/
      /*SDL_RenderClear(renerer);*/
      /**/
      /*SDL_RenderPresent(renerer);*/
    }
  }

  SDL_Log("SDL3 shutdown");

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renerer);
  SDL_Quit();

  return 0;
}

int main(const int argc, char *argv[]) {
  enum Erros { OK, WRONG_ARG, PARSE_JSON, READ_FILE };
  if (argc != 2) {
    printf("Worng Argument\n");
    return 1;
  }

  struct CPU cpu = {{0, 0, {0}, {0}, {0}, 0, 0},
                    {malloc((uint32_t)(MEM_SIZE))}};
  SDL_Log("%s \n", argv[1]);

  struct File rom = {NULL, 0};
  uint32_t err = read_file(argv[1], &rom);
  if (err != 0) {
    free(rom.data);
    free(cpu.bus.memory);
    printf("Error with reading *.gb");
    return READ_FILE;
  }

  SDL_Log("rom size: %d", rom.size);
  assert(rom.size <= (0x3FFF) * 8);
  memcpy(cpu.bus.memory, rom.data, rom.size);
  free(rom.data);

  struct File opcode = {NULL, 0};
  err = read_file("opcodes.json", &opcode);
  if (err != 0) {
    printf("Error with opcodes");
    free(opcode.data);
    free(cpu.bus.memory);
    return READ_FILE;
  }

  SDL_Log("Opcode size %u", opcode.size);

  cJSON *json = cJSON_ParseWithLength((char *)opcode.data, opcode.size);
  if (json == NULL) {
    printf("Error with json");
    free(opcode.data);
    free(cpu.bus.memory);
    return PARSE_JSON;
  }
  SDL_Log("JSON loaded");

  uint32_t t = 0;
  while (t < 10) {
    cpu_step(&cpu, json);
    t++;
  }

  free(opcode.data);
  free(cpu.bus.memory);

  return OK;
}
