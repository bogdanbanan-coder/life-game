add_library(life-game-foundation INTERFACE)

target_compile_features(life-game-foundation INTERFACE cxx_std_23)
target_include_directories(life-game-foundation INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
