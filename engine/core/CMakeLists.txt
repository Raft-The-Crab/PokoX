# PokoCore - Foundation types, diagnostics, errors, utilities

# PokoCore static library
add_library(PokoCore STATIC)

# Link against build options
target_link_libraries(PokoCore PUBLIC pokox_build_options)

# Add include directories
target_include_directories(PokoCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Source files
target_sources(PokoCore PRIVATE
    include/poko/core/Result.hpp
    src/error/Result.cpp
    include/poko/core/Logger.hpp
    src/logging/Logger.cpp
    include/poko/core/EntityId.hpp
    src/ids/EntityId.cpp
    include/poko/core/EntityPool.hpp
    src/ids/EntityPool.cpp
    include/poko/core/ResourceId.hpp
)
