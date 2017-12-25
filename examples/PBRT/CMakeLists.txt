file(GLOB PBRT_SOURCE *.cpp)
file(GLOB PBRT_INCLUDE *.h)
file(GLOB PBRT_SHADERS "${VKDISPLAY_SHADER_DIR}/PBRT/*.vert" "${VKDISPLAY_SHADER_DIR}/PBRT/*.frag")
file(GLOB PBRT_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(PBRT ${PBRT_SOURCE} ${PBRT_INCLUDE} ${PBRT_SHADERS} ${PBRT_IMAGES})
target_link_libraries(PBRT ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${PBRT_SHADERS})
source_group("Image Files" FILES ${PBRT_IMAGES})
set_target_properties(PBRT PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS PBRT DESTINATION bin)