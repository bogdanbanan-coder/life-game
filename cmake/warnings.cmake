include_guard(GLOBAL)

function(life_game_enable_warnings target_name)
    target_compile_options(
        ${target_name}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wsign-conversion
    )

    if(LIFE_GAME_WARNINGS_AS_ERRORS)
        target_compile_options(${target_name} PRIVATE -Werror)
    endif()
endfunction()
