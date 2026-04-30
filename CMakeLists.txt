cmake_minimum_required(VERSION 3.10)
set(VCPKG_TARGET_TRIPLET x64-mingw-dynamic)
project(MyGame)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 寻找包
find_package(SDL3 CONFIG REQUIRED)        # 包名: sdl3
find_package(SDL3_image CONFIG REQUIRED)  # 包名: sdl3-image
find_package(SDL3_ttf CONFIG REQUIRED)    # 包名: sdl3-ttf
find_package(SDL3_mixer CONFIG REQUIRED)  # 包名: sdl3-mixer
find_package(glm CONFIG REQUIRED)
find_package(cJSON CONFIG REQUIRED)

# 核心层源文件
set(CORE_SOURCES
    src/Core/Window.cpp
    src/Core/Renderer.cpp
    src/Core/Texture.cpp
    src/Core/Audio.cpp
    src/Core/Font.cpp
    src/Core/EventHandler.cpp
    src/Core/GameLoop.cpp
    src/Core/Scene.cpp
    src/Core/ResourceManager.cpp
    src/Core/CollisionSystem.cpp
    src/Core/Camera2D.cpp
    src/Core/Log.cpp
    src/Core/JsonLoader.cpp
    # UI系统
    src/Core/UI/UIElement.cpp
    src/Core/UI/UIPanel.cpp
    src/Core/UI/UILabel.cpp
    src/Core/UI/UIButton.cpp
    src/Core/UI/UIProgressBar.cpp
    src/Core/UI/UIManager.cpp
)

# 游戏层源文件
set(GAME_SOURCES
    src/Game/Core/Skill.cpp
    src/Game/Data/DataManager.cpp
    src/Game/World/WorldMap.cpp
    src/Game/World/BigWorldScene.cpp
    src/Game/Combat/GridMap.cpp
    src/Game/Combat/CombatUnit.cpp
    src/Game/Combat/TurnManager.cpp
    src/Game/Combat/CombatAI.cpp
    src/Game/Combat/CombatScene.cpp
    src/Game/Menu/MainMenuScene.cpp
    src/Game/Menu/SectScene.cpp
)

# 主程序
add_executable(MyGame main.cpp ${CORE_SOURCES} ${GAME_SOURCES})

# 添加核心层头文件搜索路径
target_include_directories(MyGame PRIVATE ${CMAKE_SOURCE_DIR}/src)

#指定编码
target_compile_options(MyGame PRIVATE
    -finput-charset=UTF-8
    -fexec-charset=UTF-8
)
# 链接库
target_link_libraries(MyGame
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    glm::glm
    cjson
)
target_compile_definitions(MyGame PRIVATE SDL_MAIN_HANDLED)
