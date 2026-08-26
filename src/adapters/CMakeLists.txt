add_library(life-game-adapters INTERFACE)

target_compile_features(life-game-adapters INTERFACE cxx_std_23)
target_include_directories(life-game-adapters INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(life-game-adapters INTERFACE life-game-application life-game-sqlite)
