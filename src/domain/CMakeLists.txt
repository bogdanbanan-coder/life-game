add_library(life-game-domain INTERFACE)

target_compile_features(life-game-domain INTERFACE cxx_std_23)
target_include_directories(life-game-domain INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(life-game-domain INTERFACE life-game-foundation)
