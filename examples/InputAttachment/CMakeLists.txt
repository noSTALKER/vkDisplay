file(GLOB INPUT_ATTACHMENT_SOURCE *.cpp)
file(GLOB INPUT_ATTACHMENT_INCLUDE *.h)
file(GLOB INPUT_ATTACHMENT_SHADERS "${VKDISPLAY_SHADER_DIR}/InputAttachment/*.vert" "${VKDISPLAY_SHADER_DIR}/InputAttachment/*.frag")
file(GLOB INPUT_ATTACHMENT_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR})
add_executable(InputAttachment ${INPUT_ATTACHMENT_SOURCE} ${INPUT_ATTACHMENT_INCLUDE} ${INPUT_ATTACHMENT_SHADERS} ${INPUT_ATTACHMENT_IMAGES})
target_link_libraries(InputAttachment ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${INPUT_ATTACHMENT_SHADERS})
source_group("Image Files" FILES ${INPUT_ATTACHMENT_IMAGES})
set_target_properties(InputAttachment PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS InputAttachment DESTINATION bin)