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
)

# 主程序
add_executable(MyGame main.cpp ${CORE_SOURCES})

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
