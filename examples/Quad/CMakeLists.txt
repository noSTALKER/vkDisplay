file(GLOB QUAD_SOURCE *.cpp)
file(GLOB QUAD_INCLUDE *.h)
file(GLOB QUAD_SHADERS "${VKDISPLAY_SHADER_DIR}/Quad/basic.vert" "${VKDISPLAY_SHADER_DIR}/Quad/basic.frag")


include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR})
add_executable(Quad ${QUAD_SOURCE} ${QUAD_INCLUDE} ${QUAD_SHADERS})
target_link_libraries(Quad ${Vulkan_LIBRARY} Util)

source_group("Shader Files" FILES ${QUAD_SHADERS})
set_target_properties(Quad PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS Quad DESTINATION bin)