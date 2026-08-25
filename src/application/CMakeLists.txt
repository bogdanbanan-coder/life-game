add_library(life-game-application INTERFACE)

target_compile_features(life-game-application INTERFACE cxx_std_23)
target_include_directories(life-game-application INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(life-game-application INTERFACE life-game-domain)
