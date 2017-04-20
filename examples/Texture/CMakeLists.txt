file(GLOB TEXTURE_SOURCE *.cpp)
file(GLOB TEXTURE_INCLUDE *.h)
file(GLOB TEXTURE_SHADERS "${VKDISPLAY_SHADER_DIR}/Texture/basic.vert" "${VKDISPLAY_SHADER_DIR}/Texture/basic.frag")
file(GLOB TEXTURE_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR})
add_executable(Texture ${TEXTURE_SOURCE} ${TEXTURE_INCLUDE} ${TEXTURE_SHADERS} ${TEXTURE_IMAGES})
target_link_libraries(Texture ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${TEXTURE_SHADERS})
source_group("Image Files" FILES ${TEXTURE_IMAGES})
set_target_properties(Texture PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS Texture DESTINATION bin)