cmake_minimum_required(VERSION 3.0)

set(NAME vkDisplay)
project(${NAME})

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install")
set(CMAKE_MODULE_PATH $"{CMAKE_SOURCE_DIR}/modules")
set(VKDISPLAY_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src")
set(VKDISPLAY_SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(VKDISPLAY_IMAGE_DIR "${CMAKE_SOURCE_DIR}/images")

set(GLM_INCLUDE_DIR "E:/Git/glm")

find_package(Vulkan)
add_definitions(-DGLM_FORCE_LEFT_HANDED)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

add_definitions(/fp:fast)

add_subdirectory(src)
add_subdirectory(examples)

install(DIRECTORY images DESTINATION .)
install(DIRECTORY shaders DESTINATION .)
install(DIRECTORY models DESTINATION .)