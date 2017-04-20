file(GLOB MULTI_SUBPASS_SOURCE *.cpp)
file(GLOB MULTI_SUBPASS_INCLUDE *.h)
file(GLOB MULTI_SUBPASS_RESOURCES *.frag *.vert)

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR})
add_executable(MultpleSubpass ${MULTI_SUBPASS_SOURCE} ${MULTI_SUBPASS_INCLUDE} ${MULTI_SUBPASS_RESOURCES})
target_link_libraries(MultpleSubpass ${Vulkan_LIBRARY} Util)

set_target_properties(MultpleSubpass PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS MultpleSubpass DESTINATION bin)