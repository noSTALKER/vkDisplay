file(GLOB UNIFORM_BUFFER_SOURCE *.cpp)
file(GLOB UNIFORM_BUFFER_INCLUDE *.h)
file(GLOB UNIFORM_BUFFER_SHADERS "${VKDISPLAY_SHADER_DIR}/UniformBuffer/*.vert" "${VKDISPLAY_SHADER_DIR}/UniformBuffer/*.frag")
file(GLOB UNIFORM_BUFFER_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR})
add_executable(UniformBuffer ${UNIFORM_BUFFER_SOURCE} ${UNIFORM_BUFFER_INCLUDE} ${UNIFORM_BUFFER_SHADERS} ${UNIFORM_BUFFER_IMAGES})
target_link_libraries(UniformBuffer ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${UNIFORM_BUFFER_SHADERS})
source_group("Image Files" FILES ${UNIFORM_BUFFER_IMAGES})
set_target_properties(UniformBuffer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS UniformBuffer DESTINATION bin)