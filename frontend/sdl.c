#include "../vm/vm.h"

#include <SDL2/SDL.h>

SDL_Window *sdl_window;
SDL_Renderer *sdl_renderer;
SDL_Texture *sdl_texture;

uint32_t *pixel_buffer;
uint32_t width;
uint32_t height;

void
render_init (const char *title, uint32_t w, uint32_t h, uint32_t pixel_size)
{
  SDL_Init (SDL_INIT_VIDEO);

  sdl_window = SDL_CreateWindow (title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
  sdl_renderer = SDL_CreateRenderer (sdl_window, -1, SDL_RENDERER_ACCELERATED);

  width = w / pixel_size;
  height = h / pixel_size;

  sdl_texture = SDL_CreateTexture (sdl_renderer, SDL_PIXELFORMAT_RGB888,
                                   SDL_TEXTUREACCESS_STREAMING, width, height);

  pixel_buffer = calloc (width * height, sizeof (uint32_t));
}


uint32_t
color_u16_to_u32 (uint16_t color)
{
  uint8_t a4 = (color >> 12) & 0x0F;
  uint8_t r4 = (color >> 8) & 0x0F;
  uint8_t g4 = (color >> 4) & 0x0F;
  uint8_t b4 = color & 0x0F;

  uint8_t a8 = (a4 << 4) | a4;
  uint8_t r8 = (r4 << 4) | r4;
  uint8_t g8 = (g4 << 4) | g4;
  uint8_t b8 = (b4 << 4) | b4;

  return (a8 << 24) | (r8 << 16) | (g8 << 8) | b8;
}

void
renderer_store_word (VM *vm, VM_Device *device, word address, word value)
{
  (void)vm, (void)device;
  pixel_buffer[address - 0x3000] = color_u16_to_u32 (value);
}

byte
keyboard_read_byte (VM *vm, VM_Device *device, word address)
{
  (void) vm;
  return ((byte *)device->state)[address - 0x7000];
}

int
main (int argc, char **argv)
{
  if (argc <= 1)
    {
      fprintf (stderr, "USAGE: %s <ROM>\n", argv[0]);
      return 1;
    }

  VM vm = { 0 };

  vm_create (&vm);

  VM_Device renderer = { 0 };

  renderer.read_byte = vm_default_read_byte;
  renderer.read_word = vm_default_read_word;
  renderer.store_byte = vm_default_store_byte;
  renderer.store_word = renderer_store_word;
  renderer.state = NULL;

  vm_map_device (&vm, &renderer, 0x3000, 0x7000);

  VM_Device keyboard = { 0 };

  keyboard.read_byte = keyboard_read_byte;
  keyboard.read_word = vm_default_read_word;
  keyboard.store_byte = vm_default_store_byte;
  keyboard.store_word = vm_default_store_word;
  keyboard.state = (void *)SDL_GetKeyboardState (NULL);

  vm_map_device (&vm, &keyboard, 0x7000, 0x7200);

  if (!vm_load_file (&vm, argv[1]))
    return 1;

  render_init ("", 768, 768, 6);

  uint32_t frame_start = 0;
  uint32_t frame_end = 0;

  while (!vm.halt)
    {
      if (vm_read_byte (&vm, 0x9000) == 1)
        {
          vm_store_byte (&vm, 0x9000, 0);

          SDL_Event event;
          while (SDL_PollEvent (&event))
            switch (event.type)
              {
              case SDL_QUIT:
                vm.halt = true;
                break;
              default:
                break;
              }

          for (uint32_t i = 0; i < width * height; ++i)
            pixel_buffer[i] = 0x000000;

          frame_start = SDL_GetTicks ();
        }

      vm_step (&vm);

      if (vm_read_byte (&vm, 0x9001) == 1)
        {
          vm_store_byte (&vm, 0x9001, 0);

          SDL_UpdateTexture (sdl_texture, NULL, pixel_buffer,
                             width * sizeof (uint32_t));
          SDL_RenderCopyEx (sdl_renderer, sdl_texture, NULL, NULL, 0, NULL,
                            SDL_FLIP_NONE);
          SDL_RenderPresent (sdl_renderer);

          frame_end = SDL_GetTicks ();

          // printf ("F %dms\n", frame_end - frame_start);
          SDL_Delay (1000 / 144);
        }
    }

  SDL_DestroyTexture (sdl_texture);
  SDL_DestroyRenderer (sdl_renderer);
  SDL_DestroyWindow (sdl_window);

  vm_destroy (&vm);

  return 0;
}

