/**
 * @file test_texture_loader.cpp
 * @brief Tests for TextureLoader.
 */

#include "poko/render/texture_loader.h"
#include "poko/render/texture.h"

#include <cstdio>
#include <cassert>

using namespace poko::render;

int main() {
    std::printf("PokoX Texture Loader Tests\n");
    std::printf("===========================\n\n");

    // Test 1: BMP loader with a simple inline BMP
    {
        std::printf("Test 1: BMP loader (inline BMP)... ");

        // Write a minimal 2x2 BMP file
        const char* temp_path = "test_2x2.bmp";
        FILE* f = std::fopen(temp_path, "wb");
        if (!f) {
            std::printf("FAILED (cannot write temp file)\n");
            return 1;
        }

        // BMP header (14 bytes)
        uint8_t header[14] = {
            'B', 'M',              // Signature
            0, 0, 0, 0,           // File size (placeholder)
            0, 0, 0, 0,           // Reserved
            54, 0, 0, 0           // Data offset (54)
        };

        // DIB header (40 bytes)
        uint8_t dib[40] = {0};
        dib[0] = 40;              // Header size
        dib[4] = 2; dib[5] = 0;   // Width = 2
        dib[8] = 2; dib[9] = 0;   // Height = 2
        dib[12] = 1;              // Planes = 1
        dib[14] = 24;             // Bits per pixel = 24
        dib[16] = 0; dib[17] = 0; dib[18] = 0; dib[19] = 0;  // Compression = 0
        dib[20] = 0; dib[21] = 0; dib[22] = 0; dib[23] = 0;  // Image size = 0 (uncompressed)
        dib[24] = 19; dib[25] = 11; dib[26] = 0; dib[27] = 0;  // X pixels per meter
        dib[28] = 19; dib[29] = 11; dib[30] = 0; dib[31] = 0;  // Y pixels per meter
        dib[32] = 0; dib[33] = 0; dib[34] = 0; dib[35] = 0;  // Colors used = 0
        dib[36] = 0; dib[37] = 0; dib[38] = 0; dib[39] = 0;  // Important colors = 0

        // Pixel data (2x2 BGR, bottom-up, padded to 4-byte rows)
        // Row size = (2 * 3 + 3) / 4 * 4 = 8 bytes
        uint8_t pixels[16] = {
            // Bottom row (y=0)
            255, 0, 0,    0, 255, 0, 0, 0,   // Red, Green, padding
            // Top row (y=1)
            0, 0, 255,    255, 255, 0, 0, 0   // Blue, Yellow, padding
        };

        // Update file size
        uint32_t file_size = 14 + 40 + 16;
        header[2] = file_size & 0xFF;
        header[3] = (file_size >> 8) & 0xFF;
        header[4] = (file_size >> 16) & 0xFF;
        header[5] = (file_size >> 24) & 0xFF;

        std::fwrite(header, 1, 14, f);
        std::fwrite(dib, 1, 40, f);
        std::fwrite(pixels, 1, 16, f);
        std::fclose(f);

        TextureLoader loader;
        TextureDesc desc;
        TextureLoaderError err = loader.load(temp_path, desc);

        bool ok = true;
        if (err != TextureLoaderError::Success) {
            std::printf("FAILED: %s\n", loader.get_last_error());
            ok = false;
        } else {
            if (desc.width != 2 || desc.height != 2) {
                std::printf("FAILED (dimensions: %ux%u)\n", desc.width, desc.height);
                ok = false;
            } else if (desc.format != TextureFormat::RGBA8_UNORM) {
                std::printf("FAILED (format)\n");
                ok = false;
            }
        }

        TextureLoader::cleanup(desc);
        std::remove(temp_path);

        if (!ok) {
            return 1;
        }
        std::printf("PASS\n");
    }

    // Test 2: TGA loader with a simple inline TGA
    {
        std::printf("Test 2: TGA loader (inline TGA)... ");

        const char* temp_path = "test_2x2.tga";
        FILE* f = std::fopen(temp_path, "wb");
        if (!f) {
            std::printf("FAILED (cannot write temp file)\n");
            return 1;
        }

        // TGA header (18 bytes)
        uint8_t header[18] = {0};
        header[2] = 2;              // Image type: uncompressed RGB
        header[12] = 2;             // Width = 2
        header[14] = 2;             // Height = 2
        header[16] = 24;            // Bits per pixel = 24
        header[17] = 0x20;          // Image descriptor: top-left origin

        // Pixel data (2x2 BGR)
        uint8_t pixels[12] = {
            255, 0, 0,      // Red
            0, 255, 0,      // Green
            0, 0, 255,      // Blue
            255, 255, 0     // Yellow
        };

        std::fwrite(header, 1, 18, f);
        std::fwrite(pixels, 1, 12, f);
        std::fclose(f);

        TextureLoader loader;
        TextureDesc desc;
        TextureLoaderError err = loader.load(temp_path, desc);

        bool ok = true;
        if (err != TextureLoaderError::Success) {
            std::printf("FAILED: %s\n", loader.get_last_error());
            ok = false;
        } else {
            if (desc.width != 2 || desc.height != 2) {
                std::printf("FAILED (dimensions: %ux%u)\n", desc.width, desc.height);
                ok = false;
            } else if (desc.format != TextureFormat::RGBA8_UNORM) {
                std::printf("FAILED (format)\n");
                ok = false;
            }
        }

        TextureLoader::cleanup(desc);
        std::remove(temp_path);

        if (!ok) {
            return 1;
        }
        std::printf("PASS\n");
    }

    // Test 3: Unsupported format
    {
        std::printf("Test 3: Unsupported format rejection... ");

        const char* temp_path = "test_unsupported.xyz";
        FILE* f = std::fopen(temp_path, "wb");
        if (!f) {
            std::printf("FAILED (cannot write temp file)\n");
            return 1;
        }
        std::fclose(f);

        TextureLoader loader;
        TextureDesc desc;
        TextureLoaderError err = loader.load(temp_path, desc);

        if (err != TextureLoaderError::UnsupportedFormat) {
            std::printf("FAILED (expected UnsupportedFormat)\n");
            std::remove(temp_path);
            return 1;
        }

        std::remove(temp_path);
        std::printf("PASS\n");
    }

    std::printf("\nAll texture loader tests passed.\n");
    return 0;
}
