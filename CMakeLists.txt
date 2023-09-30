cmake_minimum_required(VERSION 3.26)
project(MeatColony)
set(CMAKE_CXX_STANDARD 20)

# Thanks https://github.com/SasLuca/raylib-cmake-template/tree/master

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp") # Define PROJECT_SOURCES as a list of all source files
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/") # Define PROJECT_INCLUDE to be the path to the include directory of the project

# Declaring our executable
add_executable(${PROJECT_NAME}
        src/PartialProcessingSubsystem.hpp
        src/Manager.hpp
        src/Food.hpp
        src/Network.hpp
        src/Cell.hpp
        src/Body.hpp
        src/Egg.hpp
        src/DNA.hpp
        src/Constants.hpp
        src/Render.hpp
        src/Render.hpp
        src/Render.hpp
        src/ManagerSignals.hpp
        src/Simulation.hpp
)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
