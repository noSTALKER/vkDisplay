file(GLOB PUSH_CONSTANT_SOURCE *.cpp)
file(GLOB PUSH_CONSTANT_INCLUDE *.h)
file(GLOB PUSH_CONSTANT_SHADERS "${VKDISPLAY_SHADER_DIR}/PushConstant/*.vert" "${VKDISPLAY_SHADER_DIR}/PushConstant/*.frag")
file(GLOB PUSH_CONSTANT_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(PushConstant ${PUSH_CONSTANT_SOURCE} ${PUSH_CONSTANT_INCLUDE} ${PUSH_CONSTANT_SHADERS} ${PUSH_CONSTANT_IMAGES})
target_link_libraries(PushConstant ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${PUSH_CONSTANT_SHADERS})
source_group("Image Files" FILES ${PUSH_CONSTANT_IMAGES})
set_target_properties(PushConstant PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS PushConstant DESTINATION bin)