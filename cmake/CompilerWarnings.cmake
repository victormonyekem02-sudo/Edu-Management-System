# Applies the project warning set (see docs/06-implementation/coding-standard.md, rule CS-B1).
function(lms_set_warnings target)
    if(MSVC)
        set(warnings /W4 /permissive-)
        if(LMS_WARNINGS_AS_ERRORS)
            list(APPEND warnings /WX)
        endif()
    else()
        set(warnings
            -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
            -Wold-style-cast -Woverloaded-virtual -Wnull-dereference
            -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough)
        if(LMS_WARNINGS_AS_ERRORS)
            list(APPEND warnings -Werror)
        endif()
    endif()
    target_compile_options(${target} PRIVATE ${warnings})
endfunction()

function(lms_enable_coverage target)
    if(LMS_ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target} PRIVATE --coverage -O0 -g)
            target_link_options(${target} PUBLIC --coverage)
        else()
            message(WARNING "LMS_ENABLE_COVERAGE is only supported with GCC or Clang")
        endif()
    endif()
endfunction()
