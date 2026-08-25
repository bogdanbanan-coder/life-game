add_library(life-game-presentation INTERFACE)

target_compile_features(life-game-presentation INTERFACE cxx_std_23)
target_include_directories(life-game-presentation INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(life-game-presentation INTERFACE life-game-application raylib life-game-raygui)
