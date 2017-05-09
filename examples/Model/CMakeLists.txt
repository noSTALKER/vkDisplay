file(GLOB MODEL_SOURCE *.cpp)
file(GLOB MODEL_INCLUDE *.h)
file(GLOB MODEL_SHADERS "${VKDISPLAY_SHADER_DIR}/Model/*.vert" "${VKDISPLAY_SHADER_DIR}/Model/*.frag")
file(GLOB MODEL_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(Model ${MODEL_SOURCE} ${MODEL_INCLUDE} ${MODEL_SHADERS} ${MODEL_IMAGES})
target_link_libraries(Model ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${MODEL_SHADERS})
source_group("Image Files" FILES ${MODEL_IMAGES})
set_target_properties(Model PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS Model DESTINATION bin)