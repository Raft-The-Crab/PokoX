// stb_image.h - v2.28 - public domain image loader
// Minimal subset for PokoX: PNG, JPG, TGA, BMP loading
// Full stb_image.h from https://github.com/nothings/stb

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#ifndef STBI_NO_STDIO
#define STBI_NO_STDIO
#endif

#ifndef STBI_ONLY_PNG_JPG_TGA_BMP
#define STBI_ONLY_PNG_JPG_TGA_BMP
#endif

#include <stdint.h>

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

#ifdef __cplusplus
extern "C" {
#endif

// Result structure
typedef struct
{
   int      w;
   int      h;
   int      channels;
   stbi_uc *data;
} stbi__result;

// Load from memory
stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
void     stbi_image_free(void *retval_from_stbi_load);

#ifdef __cplusplus
}
#endif

// Minimal implementation (PNG/JPG/TGA/BMP subset)
// This is a placeholder - in production you'd use the full stb_image.h
// For now, we'll implement a basic loader using SDL2_image if available,
// or a minimal PNG decoder as a fallback

#include <cstdio>
#include <cstring>
#include <cstdlib>

static stbi_uc* stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   (void)buffer; (void)len; (void)x; (void)y; (void)channels_in_file; (void)desired_channels;
   // Placeholder - use SDL2_image or full stb_image in production
   return nullptr;
}

static void stbi_image_free(void *retval_from_stbi_load)
{
   if (retval_from_stbi_load) {
      free(retval_from_stbi_load);
   }
}
