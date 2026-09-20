add_library(kfighter_core STATIC
    src/entity.cpp
    src/collision.cpp
    src/world.cpp
)

target_include_directories(kfighter_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(kfighter_core PUBLIC cxx_std_20)
