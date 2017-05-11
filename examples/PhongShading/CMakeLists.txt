file(GLOB PHONG_SHADING_SOURCE *.cpp)
file(GLOB PHONG_SHADING_INCLUDE *.h)
file(GLOB PHONG_SHADING_SHADERS "${VKDISPLAY_SHADER_DIR}/PhongShading/*.vert" "${VKDISPLAY_SHADER_DIR}/PhongShading/*.frag")
file(GLOB PHONG_SHADING_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(PhongShading ${PHONG_SHADING_SOURCE} ${PHONG_SHADING_INCLUDE} ${PHONG_SHADING_SHADERS} ${PHONG_SHADING_IMAGES})
target_link_libraries(PhongShading ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${PHONG_SHADING_SHADERS})
source_group("Image Files" FILES ${PHONG_SHADING_IMAGES})
set_target_properties(PhongShading PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS PhongShading DESTINATION bin)