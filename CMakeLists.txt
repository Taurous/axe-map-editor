cmake_minimum_required(VERSION 3.16.3)

project(axe-map-editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_FLAGS "-g -Wall -D_DEBUG")

if(WIN32)

link_directories(C:/libraries/allegro/lib)
add_executable(${PROJECT_NAME}
    src/lerp.cpp
    src/util.cpp
    src/view.cpp
    src/input.cpp
    src/map.cpp
    src/map_editor.cpp
    src/main.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE C:/libraries/allegro/include include)
target_link_libraries(${PROJECT_NAME} allegro allegro_main allegro_primitives allegro_font allegro_ttf allegro_image allegro_color allegro_dialog)

else()

add_executable(${PROJECT_NAME}
    src/lerp.cpp
    src/util.cpp
    src/view.cpp
    src/input.cpp
    src/gui.cpp
    src/map.cpp
    src/map_editor.cpp
    src/main.cpp
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(ALLEGRO5 REQUIRED allegro-5 allegro_main-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_color-5 allegro_dialog-5)
pkg_check_modules(CURLPP REQUIRED curlpp)

add_library(imgui
    ../imgui/imgui.cpp
    ../imgui/imgui_tables.cpp
    ../imgui/imgui_draw.cpp
    ../imgui/imgui_widgets.cpp
    ../imgui/backends/imgui_impl_allegro5.cpp
    ../imgui/imgui_demo.cpp
)

target_include_directories(imgui PUBLIC ${ALLEGRO5_LIBRARIES})
target_include_directories(imgui PUBLIC ../imgui/ ../imgui/backends/)

target_link_libraries(${PROJECT_NAME}
    ${ALLEGRO5_LIBRARIES}
    ${CURLPP_LIBRARIES}
    imgui
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${ALLEGRO5_INCLUDE_DIRS}
    ${CURLPP_INCLUDE_DIRS}
    ../imgui/
    ../imgui/backends/
)

endif()