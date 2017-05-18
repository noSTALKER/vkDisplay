file(GLOB MSAA_SOURCE *.cpp)
file(GLOB MSAA_INCLUDE *.h)
file(GLOB MSAA_SHADERS "${VKDISPLAY_SHADER_DIR}/MSAA/*.vert" "${VKDISPLAY_SHADER_DIR}/MSAA/*.frag")
file(GLOB MSAA_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(MSAA ${MSAA_SOURCE} ${MSAA_INCLUDE} ${MSAA_SHADERS} ${MSAA_IMAGES})
target_link_libraries(MSAA ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${MSAA_SHADERS})
source_group("Image Files" FILES ${MSAA_IMAGES})
set_target_properties(MSAA PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS MSAA DESTINATION bin)