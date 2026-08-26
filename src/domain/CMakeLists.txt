add_library(
    life-game-domain
    STATIC
        field/field-dimensions.cpp
        field/field.cpp
)

target_compile_features(life-game-domain PUBLIC cxx_std_23)
target_include_directories(life-game-domain PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(life-game-domain PUBLIC life-game-foundation)
life_game_enable_warnings(life-game-domain)
life_game_enable_sanitizers(life-game-domain)
