#include "vm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

uint32_t colors[] = {
  0xFF0000, 0x00FF00, 0x0000FF,
  0xFFFF00, 0xFF00FF, 0x00FFFF,
  0x800000, 0x808000, 0x008000,
  0x800080, 0x008080, 0x000080,
  0xC0C0C0, 0x808080, 0x404040,
  0xFFA500, 0xA52A2A, 0x8A2BE2,
  0x5F9EA0, 0x7FFF00, 0xD2691E,
  0xDC143C, 0x00CED1, 0x9400D3,
  0xFF1493, 0x00BFFF, 0x696969,
  0x1E90FF, 0xB22222, 0x228B22,
  0xFFFAF0, 0xDCDCDC, 0xF8F8FF,
  0xFFD700, 0xDAA520, 0xADFF2F,
  0xF0FFF0, 0xFF69B4, 0xCD5C5C,
  0x4B0082, 0xFFFFF0, 0xF0E68C,
  0xE6E6FA, 0xFFF0F5, 0x7CFC00,
  0xFFFACD, 0xADD8E6, 0xF08080,
  0xE0FFFF, 0xFAFAD2, 0xD3D3D3,
  0x90EE90, 0xFFB6C1, 0xFFA07A,
  0x20B2AA, 0x87CEFA, 0x778899,
  0xB0C4DE, 0xFFFFE0, 0x32CD32,
  0xFAF0E6, 0x800000, 0x66CDAA,
  0xBA55D3, 0x9370DB, 0x3CB371,
  0x7B68EE, 0x00FA9A, 0x48D1CC,
  0xC71585, 0x191970, 0xF5FFFA,
  0xFFE4E1, 0xFFE4B5, 0xFFDEAD,
  0x000080, 0xFDF5E6, 0x6B8E23,
  0xFFA500, 0xFF4500, 0xDA70D6,
  0xEEE8AA, 0x98FB98, 0xAFEEEE,
  0xDB7093, 0xFFEFD5, 0xFFDAB9,
  0xCD853F, 0xFFC0CB, 0xDDA0DD,
  0xB0E0E6, 0xBC8F8F, 0x4169E1,
  0x8B4513, 0xFA8072, 0xF4A460,
  0x2E8B57, 0xFFF5EE, 0xA0522D,
  0xC0C0C0, 0x87CEEB, 0x6A5ACD,
  0x708090, 0xFFFAFA, 0x00FF7F,
  0x4682B4, 0xD2B48C, 0x008080,
  0xD8BFD8, 0xFF6347, 0x40E0D0,
  0xEE82EE, 0xF5DEB3, 0xFFFFFF
};

static inline void
load_file (VM *vm, const char *path)
{
  FILE *file = fopen (path, "rb");

  fseek(file, 0, SEEK_END);
  long nmemb = ftell (file);
  rewind(file);

  byte *bytecode = malloc (nmemb);
  size_t nread = fread (bytecode, 1, nmemb, file);

  fclose (file);

  memcpy (vm->memory, bytecode, nread);
  free (bytecode);
}

static struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint32_t *pixel_buffer;
  uint32_t width, height;
} state;

static size_t buffer_sz;

void
render_init (const char *title, uint32_t width, uint32_t height,
             uint32_t pixel_size)
{
  SDL_Init (SDL_INIT_VIDEO);
  state.window = SDL_CreateWindow (title, SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, width,
                                     height, SDL_WINDOW_SHOWN);
  state.renderer = SDL_CreateRenderer (state.window, -1,
                                       SDL_RENDERER_ACCELERATED);

  state.width = width / pixel_size;
  state.height = height / pixel_size;

  state.texture = SDL_CreateTexture (
      state.renderer, SDL_PIXELFORMAT_RGB888,
      SDL_TEXTUREACCESS_STREAMING, state.width, state.height);

  buffer_sz = state.width * state.height * sizeof (uint32_t);
  state.pixel_buffer = malloc (buffer_sz);
}

void
render_clear ()
{
  memset (state.pixel_buffer, 0x00, buffer_sz);
}

void
render_present (int64_t flags)
{
  SDL_UpdateTexture (state.texture, NULL, state.pixel_buffer,
                     state.width * sizeof (uint32_t));
  SDL_RenderCopyEx (state.renderer, state.texture, NULL, NULL, 0,
                    NULL, flags);
  SDL_RenderPresent (state.renderer);
}

static inline uint8_t
point_in_bounds (float x, float y)
{
  return (x >= 0 && x < state.width)
         && (y >= 0 && y < state.height);
}

static inline uint32_t
point_to_index (float x, float y, uint32_t stride)
{
  return (int32_t)x + (int32_t)y * stride;
}

void
draw_point (word index, uint32_t color)
{
  state.pixel_buffer[index] = color;
}

byte
renderer_load_byte (VM *vm, VM_Device *device, word address)
{
  (void) vm, (void) device, (void) address;
  return 0;
}

uint32_t
convertColor (uint16_t color)
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
renderer_store_byte (VM *vm, VM_Device *device, word address, byte value)
{
  (void) vm, (void) device;
  word index = address - 0x3000;
  draw_point (index, colors[value]);
}

word
renderer_load_word (VM *vm, VM_Device *device, word address)
{
  (void) vm, (void) device, (void) address;
  return 0;
}

void
renderer_store_word (VM *vm, VM_Device *device, word address, word value)
{
  (void) vm, (void) device, (void) address, (void) value;
  word index = address - 0x3000;
  draw_point (index, convertColor (value));
}

byte
keyboard_load_byte (VM *vm, VM_Device *device, word address)
{
  (void) vm;
  return ((byte *)device->state)[address - 0x7000];
}

void
keyboard_store_byte (VM *vm, VM_Device *device, word address, byte value)
{
  (void) vm, (void) device, (void) address, (void) value;
}

word
keyboard_load_word (VM *vm, VM_Device *device, word address)
{
  (void) vm, (void) device, (void) address;
  return 0;
}

void
keyboard_store_word (VM *vm, VM_Device *device, word address, word value)
{
  (void) vm, (void) device, (void) address, (void) value;
}

int
main (int argc, char *argv[argc])
{
  VM vm = { 0 };
  vm_create (&vm);

  load_file (&vm, argv[1]);

  VM_Device renderer = {0};
  renderer.load_byte = renderer_load_byte;
  renderer.store_byte = renderer_store_byte;
  renderer.load_word = renderer_load_word;
  renderer.store_word = renderer_store_word;
  vm_map_device (&vm, &renderer, 0x3000, 0x7000);

  VM_Device keyboard = {0};
  keyboard.load_byte = keyboard_load_byte;
  keyboard.store_byte = keyboard_store_byte;
  keyboard.load_word = keyboard_load_word;
  keyboard.store_word = keyboard_store_word;
  keyboard.state = (void *)SDL_GetKeyboardState(NULL);
  vm_map_device (&vm, &keyboard, 0x7000, 0x7100);

  render_init ("16bit-vm", 768, 768, 6);
  // render_init ("16bit-vm", 128, 128, 1);

  printf ("%dx%d\n", state.width, state.height);
  printf ("%ld colors\n", VM_ARRAY_SIZE (colors));


  while (!vm.halt)
    {
      if (vm.memory[0xFFFA])
        {
          SDL_Event event;
          while (SDL_PollEvent (&event))
            {
              switch (event.type)
                {
                case SDL_QUIT:
                  goto quit;
                default:
                  break;
                }
            }

          render_clear ();
          vm.memory[0xFFFA] = 0;
        }

      // for (byte i = 0; i < VM_REGISTER_COUNT; ++i)
      //   vm_view_register (&vm, i);
     
      // vm_view_memory (&vm, *vm.ip, 4, 12, true);

      // if (getc (stdin) != '\n')
      //   continue;

      // printf ("%d\n", *vm.sp);
      vm_step (&vm);

      if (vm.memory[0xFFFB])
        {
          render_present (SDL_FLIP_NONE);
          vm.memory[0xFFFB] = 0;
          SDL_Delay (1000 / 60);
        }
    }

quit:
  return 0;
}

