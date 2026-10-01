cmake_minimum_required(VERSION 3.10)

project(
    NBLint
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LUAU_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_WEB OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_FUZZER OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    Luau
    GIT_REPOSITORY https://github.com/luau-lang/luau.git
    GIT_TAG c0e346e
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Luau)

# idk why but Luau's packages always raises some warns
# maybe `clang-tidy` is too difficult to use or maybe they follow the philosophy "codestyle? never heard of him"
# anyway here we go. we need to disable all the internal Luau's warns with this wheelchair
foreach(tgt Luau.Ast Luau.Analysis Luau.Compiler Luau.VM Luau.CodeGen Luau.Config)
    if(TARGET ${tgt})
        get_target_property(_incs ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
        if(_incs)
            set_target_properties(${tgt} PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_incs}")
        endif()
    endif()
endforeach()

add_executable(
    linter
    src/main.cpp
    src/analysis/checker.cpp
    src/analysis/visitor.cpp
    src/analysis/fs_resolver.cpp
    src/analysis/cfg_resolver.cpp
    src/utils/files.cpp
    src/utils/logs.cpp
)

target_include_directories(
    linter
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
    linter
    PRIVATE
    Luau.Analysis
)

if (MSVC)
    target_compile_options(
        linter
        PRIVATE
        /W4
    )
else()
    target_compile_options(
        linter
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()