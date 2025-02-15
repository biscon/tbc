cmake_minimum_required(VERSION 3.0)
project(sandbox CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adding Raylib
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

# Add these flags for debug builds
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SANITIZER_FLAGS "-fsanitize=address -g -O1 -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/sources/*.cpp") # Define PROJECT_SOURCES as a list of all source files
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/") # Define PROJECT_INCLUDE to be the path to the include directory of the project

# Declaring our executable
add_executable(${PROJECT_NAME}
        sources/character/Character.h
        sources/combat/Combat.cpp
        sources/combat/Combat.h
        sources/util/Random.cpp
        sources/util/Random.h
        sources/ui/CombatScreen.cpp
        sources/ui/CombatScreen.h
        sources/raygui.h
        sources/graphics/Animation.cpp
        sources/graphics/Animation.h
        sources/reasings.h
        sources/combat/SkillRunner.cpp
        sources/combat/SkillRunner.h
        sources/graphics/SpriteAnimation.cpp
        sources/graphics/SpriteAnimation.h
        sources/ui/Grid.cpp
        sources/ui/Grid.h
        sources/ui/UI.cpp
        sources/ui/UI.h
        sources/util/MathUtil.cpp
        sources/util/MathUtil.h
        sources/combat/CombatState.cpp
        sources/combat/CombatState.h
        sources/ai/Ai.cpp
        sources/ai/Ai.h
        sources/ai/FighterAi.cpp
        sources/ai/FighterAi.h
        sources/util/cute_tiled.h
        sources/ai/PathFinding.cpp
        sources/ai/PathFinding.h
        sources/graphics/TileMap.cpp
        sources/graphics/TileMap.h
        sources/graphics/BloodPool.cpp
        sources/graphics/BloodPool.h
        sources/graphics/ParticleSystem.cpp
        sources/graphics/ParticleSystem.h
        sources/character/StatusEffect.cpp
        sources/character/StatusEffect.h
        sources/character/Skill.cpp
        sources/character/Skill.h
        sources/combat/StatusEffectRunner.cpp
        sources/combat/StatusEffectRunner.h
        sources/combat/CombatAnimation.cpp
        sources/combat/CombatAnimation.h
        sources/audio/SoundEffect.cpp
        sources/audio/SoundEffect.h
        sources/graphics/CharacterSprite.cpp
        sources/graphics/CharacterSprite.h
        sources/graphics/SpriteSheet.cpp
        sources/graphics/SpriteSheet.h
        sources/character/Weapon.cpp
        sources/character/Weapon.h
        sources/GameMode.cpp
        sources/GameMode.h
        sources/CombatGameMode.cpp
        sources/CombatGameMode.h
        sources/DungeonGameMode.cpp
        sources/DungeonGameMode.h
        sources/graphics/DungeonRenderer.cpp
        sources/graphics/DungeonRenderer.h
        sources/dungeon/DungeonMap.cpp
        sources/dungeon/DungeonMap.h
        sources/dungeon/Lighting.cpp
        sources/dungeon/Lighting.h
        sources/ui/MiniMap.cpp
        sources/ui/MiniMap.h
        sources/dungeon/DungeonState.cpp
        sources/dungeon/DungeonState.h
        sources/game/GameState.cpp
        sources/game/GameState.h
        sources/MenuGameMode.cpp
        sources/MenuGameMode.h
)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Setting ASSETS_PATH
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
#target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./assets") # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable