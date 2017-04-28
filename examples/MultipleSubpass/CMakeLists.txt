file(GLOB MULTI_SUBPASS_SOURCE *.cpp)
file(GLOB MULTI_SUBPASS_INCLUDE *.h)
file(GLOB MULTI_SUBPASS_SHADERS "${VKDISPLAY_SHADER_DIR}/MultipleSubpass/*.vert" "${VKDISPLAY_SHADER_DIR}/MultipleSubpass/*.frag")
file(GLOB MULTI_SUBPASS_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR})
add_executable(MultipleSubpass ${MULTI_SUBPASS_SOURCE} ${MULTI_SUBPASS_INCLUDE} ${MULTI_SUBPASS_SHADERS} ${MULTI_SUBPASS_IMAGES})
target_link_libraries(MultipleSubpass ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${MULTI_SUBPASS_SHADERS})
source_group("Image Files" FILES ${MULTI_SUBPASS_IMAGES})
set_target_properties(MultipleSubpass PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS MultipleSubpass DESTINATION bin)