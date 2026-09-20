add_executable(kfighter_desktop src/main.cpp)

target_link_libraries(kfighter_desktop PRIVATE
    kfighter_core
    SDL2::SDL2
    SDL2::SDL2main
)

target_compile_features(kfighter_desktop PRIVATE cxx_std_20)

if(WIN32)
    # Copy SDL2.dll next to executable if building shared
    if(TARGET SDL2)
        add_custom_command(TARGET kfighter_desktop POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2>
            $<TARGET_FILE_DIR:kfighter_desktop>
        )
    endif()
endif()
