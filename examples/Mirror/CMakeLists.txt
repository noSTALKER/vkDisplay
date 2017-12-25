file(GLOB MIRROR_SOURCE *.cpp)
file(GLOB MIRROR_INCLUDE *.h)
file(GLOB MIRROR_SHADERS "${VKDISPLAY_SHADER_DIR}/Mirror/*.vert" "${VKDISPLAY_SHADER_DIR}/Mirror/*.frag")
file(GLOB MIRROR_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(Mirror ${MIRROR_SOURCE} ${MIRROR_INCLUDE} ${MIRROR_SHADERS} ${MIRROR_IMAGES})
target_link_libraries(Mirror ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${MIRROR_SHADERS})
source_group("Image Files" FILES ${MIRROR_IMAGES})
set_target_properties(Mirror PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS Mirror DESTINATION bin)