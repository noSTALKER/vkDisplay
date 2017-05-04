file(GLOB INSTANCING_SOURCE *.cpp)
file(GLOB INSTANCING_INCLUDE *.h)
file(GLOB INSTANCING_SHADERS "${VKDISPLAY_SHADER_DIR}/Instancing/*.vert" "${VKDISPLAY_SHADER_DIR}/Instancing/*.frag")
file(GLOB INSTANCING_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(Instancing ${INSTANCING_SOURCE} ${INSTANCING_INCLUDE} ${INSTANCING_SHADERS} ${INSTANCING_IMAGES})
target_link_libraries(Instancing ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${INSTANCING_SHADERS})
source_group("Image Files" FILES ${INSTANCING_IMAGES})
set_target_properties(Instancing PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS Instancing DESTINATION bin)