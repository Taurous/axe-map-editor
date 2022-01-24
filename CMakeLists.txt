cmake_minimum_required(VERSION 3.16.3)

project(axe-map-editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_FLAGS "-g -Wall -D_DEBUG")

find_package(PkgConfig REQUIRED)

add_executable(${PROJECT_NAME}
    src/util.cpp
    src/view.cpp
    src/input.cpp
    src/map.cpp
    src/state_machine.cpp
    src/file_chooser_state.cpp
    src/editor_state.cpp
    src/main.cpp
)

pkg_check_modules(ALLEGRO5 REQUIRED allegro-5 allegro_main-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_color-5 allegro_dialog-5)
target_link_libraries(${PROJECT_NAME} ${ALLEGRO5_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    include
    ${ALLEGRO5_INCLUDE_DIRS}
)