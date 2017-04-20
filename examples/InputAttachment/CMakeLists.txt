file(GLOB INPUT_ATTACHMENT_SOURCE *.cpp)
file(GLOB INPUT_ATTACHMENT_INCLUDE *.h)
file(GLOB INPUT_ATTACHMENT_RESOURCES *.frag *.vert)

include_directories(${Vulkan_INCLUDE_DIR} ${VKDISPLAY_INCLUDE_DIR})
add_executable(InputAttachment ${INPUT_ATTACHMENT_SOURCE} ${INPUT_ATTACHMENT_INCLUDE} ${INPUT_ATTACHMENT_RESOURCES})
target_link_libraries(InputAttachment ${Vulkan_LIBRARY} Util)

set_target_properties(InputAttachment PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS InputAttachment DESTINATION bin)