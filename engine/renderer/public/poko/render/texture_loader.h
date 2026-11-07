/**
 * @file texture_loader.h
 * @brief Texture image loader (PNG, JPG, TGA, BMP via stb_image).
 *
 * Loads image files into raw pixel data for use with Texture creation.
 * Supports common formats through stb_image single-header library.
 *
 * Usage example:
 * @code
 * TextureLoader loader;
 * TextureDesc desc;
 * if (loader.load("texture.png", desc)) {
 *     Texture texture;
 *     texture.initialize(device, desc);
 *     loader.cleanup(desc);  // Free the loaded pixel data
 * }
 * @endcode
 */

#ifndef POKO_RENDER_TEXTURE_LOADER_H
#define POKO_RENDER_TEXTURE_LOADER_H

#include "device.h"
#include "texture.h"
#include <cstdint>
#include <string>

namespace poko::render {

/**
 * @brief Texture loader error codes.
 */
enum class TextureLoaderError : int32_t {
    Success = 0,
    Unknown = -1,
    FileNotFound = -2,
    InvalidFile = -3,
    UnsupportedFormat = -4,
    AllocationFailed = -5
};

/**
 * @brief Texture loader.
 */
class TextureLoader {
public:
    TextureLoader();
    ~TextureLoader() = default;

    /**
     * @brief Loads an image file and populates a TextureDesc.
     *
     * The returned TextureDesc owns dynamically allocated pixel data; the
     * caller must free it after use (TextureLoader::cleanup() helper provided).
     *
     * @param filepath Path to the image file.
     * @param out_desc Receives the texture description.
     * @return TextureLoaderError::Success on success, error code on failure.
     */
    TextureLoaderError load(const char* filepath, TextureDesc& out_desc);

    /**
     * @brief Cleans up dynamically allocated pixel data in a TextureDesc.
     *
     * Call this after the data has been passed to Texture::initialize()
     * if you no longer need the intermediate data.
     *
     * @param desc The TextureDesc to clean up.
     */
    static void cleanup(TextureDesc& desc);

    /**
     * @brief Returns the last error message.
     */
    const char* get_last_error() const;

private:
    std::string last_error_;
};

} // namespace poko::render

#endif // POKO_RENDER_TEXTURE_LOADER_H
