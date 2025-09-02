cmake_minimum_required(VERSION 3.16)
project(ClothSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

pkg_check_modules(GLFW3 REQUIRED glfw3)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui ${GLFW3_LIBRARIES})
target_compile_options(imgui PRIVATE ${GLFW3_CFLAGS_OTHER})

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${IMGUI_DIR}/backends
    ${GLFW3_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    ${GLFW3_LIBRARIES}
    GLEW::GLEW
    imgui
)

target_compile_options(${PROJECT_NAME} PRIVATE ${GLFW3_CFLAGS_OTHER})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -O2)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

if(EXISTS ${CMAKE_SOURCE_DIR}/shaders)
    file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
endif()
