file(GLOB NORMAL_MAPPING_SOURCE *.cpp)
file(GLOB NORMAL_MAPPING_INCLUDE *.h)
file(GLOB NORMAL_MAPPING_SHADERS "${VKDISPLAY_SHADER_DIR}/NormalMapping/*.vert" "${VKDISPLAY_SHADER_DIR}/NormalMapping/*.frag")
file(GLOB NORMAL_MAPPING_IMAGES "${VKDISPLAY_IMAGE_DIR}/sample.jpg")

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
add_executable(NormalMapping ${NORMAL_MAPPING_SOURCE} ${NORMAL_MAPPING_INCLUDE} ${NORMAL_MAPPING_SHADERS} ${NORMAL_MAPPING_IMAGES})
target_link_libraries(NormalMapping ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${NORMAL_MAPPING_SHADERS})
source_group("Image Files" FILES ${NORMAL_MAPPING_IMAGES})
set_target_properties(NormalMapping PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS NormalMapping DESTINATION bin)