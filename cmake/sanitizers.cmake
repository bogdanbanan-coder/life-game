include_guard(GLOBAL)

function(life_game_enable_sanitizers target_name)
    if(LIFE_GAME_ENABLE_ASAN)
        target_compile_options(${target_name} PRIVATE -fsanitize=address)
        target_link_options(${target_name} PRIVATE -fsanitize=address)
    endif()

    if(LIFE_GAME_ENABLE_UBSAN)
        target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
        target_link_options(${target_name} PRIVATE -fsanitize=undefined)
    endif()
endfunction()
