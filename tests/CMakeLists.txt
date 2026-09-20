add_executable(test_collision src/test_collision.cpp)
target_link_libraries(test_collision PRIVATE kfighter_core)
target_compile_features(test_collision PRIVATE cxx_std_20)
add_test(NAME CollisionTest COMMAND test_collision)

add_executable(test_physics src/test_physics.cpp)
target_link_libraries(test_physics PRIVATE kfighter_core)
target_compile_features(test_physics PRIVATE cxx_std_20)
add_test(NAME PhysicsTest COMMAND test_physics)
