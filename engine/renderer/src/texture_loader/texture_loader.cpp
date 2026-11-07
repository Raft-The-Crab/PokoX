/**
 * @file texture_loader.cpp
 * @brief Texture loader implementation (BMP and TGA).
 *
 * Implements a minimal BMP and TGA loader for testing purposes.
 * Production should use stb_image or SDL2_image for full format support.
 */

#include "poko/render/texture_loader.h"
#include "poko/render/texture.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace poko::render {

namespace {

// Minimal BMP loader (24/32-bit uncompressed)
bool load_bmp(const char* filepath, TextureDesc& out_desc, std::string& error) {
    FILE* f = std::fopen(filepath, "rb");
    if (!f) {
        error = "Failed to open BMP file";
        return false;
    }

    // Read BMP header
    uint8_t header[54];
    if (std::fread(header, 1, 54, f) != 54) {
        std::fclose(f);
        error = "Invalid BMP header";
        return false;
    }

    // Check signature "BM"
    if (header[0] != 'B' || header[1] != 'M') {
        std::fclose(f);
        error = "Invalid BMP signature";
        return false;
    }

    // Extract info (little-endian)
    int32_t width = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
    int32_t height = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
    uint16_t bits_per_pixel = header[28] | (header[29] << 8);

    // Extract data offset (little-endian)
    int32_t data_offset = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);

    if (bits_per_pixel != 24 && bits_per_pixel != 32) {
        std::fclose(f);
        error = "Only 24/32-bit BMP supported";
        return false;
    }

    int32_t row_size = ((width * bits_per_pixel + 31) / 32) * 4;
    int32_t data_size = row_size * std::abs(height);

    std::vector<uint8_t> data(data_size);
    std::fseek(f, data_offset, SEEK_SET);
    if (std::fread(data.data(), 1, data_size, f) != static_cast<size_t>(data_size)) {
        std::fclose(f);
        error = "Failed to read BMP data";
        return false;
    }
    std::fclose(f);

    // Convert to RGBA (BGR -> RGB, add alpha)
    int channels = 4;
    int total_pixels = width * std::abs(height);
    uint8_t* rgba = new uint8_t[total_pixels * channels];

    for (int y = 0; y < std::abs(height); ++y) {
        int src_y = (height > 0) ? (height - 1 - y) : y;  // BMP is bottom-up
        const uint8_t* src_row = data.data() + src_y * row_size;
        uint8_t* dst_row = rgba + y * width * channels;

        for (int x = 0; x < width; ++x) {
            const uint8_t* src = src_row + x * (bits_per_pixel / 8);
            uint8_t* dst = dst_row + x * channels;

            dst[0] = src[2];  // R
            dst[1] = src[1];  // G
            dst[2] = src[0];  // B
            dst[3] = (bits_per_pixel == 32) ? src[3] : 255;  // A
        }
    }

    out_desc.data = rgba;
    out_desc.width = static_cast<uint32_t>(width);
    out_desc.height = static_cast<uint32_t>(std::abs(height));
    out_desc.format = TextureFormat::RGBA8_UNORM;
    out_desc.mip_levels = 1;
    out_desc.name = filepath;

    return true;
}

// Minimal TGA loader (uncompressed 24/32-bit)
bool load_tga(const char* filepath, TextureDesc& out_desc, std::string& error) {
    FILE* f = std::fopen(filepath, "rb");
    if (!f) {
        error = "Failed to open TGA file";
        return false;
    }

    uint8_t header[18];
    if (std::fread(header, 1, 18, f) != 18) {
        std::fclose(f);
        error = "Invalid TGA header";
        return false;
    }

    // Check image type (2 = uncompressed RGB, 3 = grayscale)
    uint8_t image_type = header[2];
    if (image_type != 2 && image_type != 3) {
        std::fclose(f);
        error = "Only uncompressed RGB/gray TGA supported";
        return false;
    }

    // Read width/height as little-endian 16-bit
    int16_t width = header[12] | (header[13] << 8);
    int16_t height = header[14] | (header[15] << 8);
    uint8_t bits_per_pixel = header[16];

    if (bits_per_pixel != 24 && bits_per_pixel != 32) {
        std::fclose(f);
        error = "Only 24/32-bit TGA supported";
        return false;
    }

    int channels = bits_per_pixel / 8;
    int total_pixels = width * height;
    int data_size = total_pixels * channels;

    std::vector<uint8_t> data(data_size);
    if (std::fread(data.data(), 1, data_size, f) != static_cast<size_t>(data_size)) {
        std::fclose(f);
        error = "Failed to read TGA data";
        return false;
    }
    std::fclose(f);

    // Convert to RGBA if needed (BGR -> RGB)
    uint8_t* rgba = new uint8_t[total_pixels * 4];

    for (int i = 0; i < total_pixels; ++i) {
        const uint8_t* src = data.data() + i * channels;
        uint8_t* dst = rgba + i * 4;

        dst[0] = src[2];  // R
        dst[1] = src[1];  // G
        dst[2] = src[0];  // B
        dst[3] = (channels == 4) ? src[3] : 255;  // A
    }

    out_desc.data = rgba;
    out_desc.width = static_cast<uint32_t>(width);
    out_desc.height = static_cast<uint32_t>(height);
    out_desc.format = TextureFormat::RGBA8_UNORM;
    out_desc.mip_levels = 1;
    out_desc.name = filepath;

    return true;
}

} // namespace

TextureLoader::TextureLoader() {}

TextureLoaderError TextureLoader::load(const char* filepath, TextureDesc& out_desc) {
    out_desc = TextureDesc{};  // Zero-initialize

    if (!filepath) {
        last_error_ = "Filepath is null";
        return TextureLoaderError::FileNotFound;
    }

    std::string path_str(filepath);
    std::string ext;

    // Extract extension
    size_t dot = path_str.rfind('.');
    if (dot != std::string::npos) {
        ext = path_str.substr(dot);
        for (char& c : ext) {
            c = std::tolower(c);
        }
    }

    if (ext == ".bmp") {
        if (!load_bmp(filepath, out_desc, last_error_)) {
            return TextureLoaderError::InvalidFile;
        }
    } else if (ext == ".tga") {
        if (!load_tga(filepath, out_desc, last_error_)) {
            return TextureLoaderError::InvalidFile;
        }
    } else {
        last_error_ = "Unsupported format (only BMP and TGA supported)";
        return TextureLoaderError::UnsupportedFormat;
    }

    return TextureLoaderError::Success;
}

void TextureLoader::cleanup(TextureDesc& desc) {
    if (desc.data) {
        delete[] static_cast<const uint8_t*>(desc.data);
        desc.data = nullptr;
    }
}

const char* TextureLoader::get_last_error() const {
    return last_error_.c_str();
}

} // namespace poko::render
