# PokoPlatform - Platform-specific code boundary

# PokoPlatform static library
add_library(PokoPlatform STATIC)

# Link against build options and PokoCore
target_link_libraries(PokoPlatform PUBLIC pokox_build_options PokoCore)

# Add include directories
target_include_directories(PokoPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Platform-specific source files
target_sources(PokoPlatform
    PRIVATE
        src/windows/WindowsWindow.cpp
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(PokoPlatform PUBLIC user32 gdi32)
endif()
