file(GLOB UTIL_SOURCE *.cpp)
file(GLOB UTIL_INCLUDE *.h)

include_directories(${Vulkan_INCLUDE_DIR})
add_library(Util SHARED ${UTIL_SOURCE} ${UTIL_INCLUDE})
target_link_libraries(Util ${Vulkan_LIBRARY})

install(TARGETS Util DESTINATION bin)