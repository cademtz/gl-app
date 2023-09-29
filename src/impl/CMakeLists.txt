include(impl_list.cmake)

#####################################################
# Set default implementations for different platforms
#####################################################

set(THREADING_DEFAULT "NONE")
set(RESOURCE_DEFAULT "FILESYSTEM")
if (EMSCRIPTEN)
    set(THREADING_DEFAULT "EMSCRIPTEN")
    set(RESOURCE_DEFAULT "EMSCRIPTEN")
endif()

################################
# Define implementation settings
################################

define_impl_list("WINDOW" OPTIONS "GLFW" DEFAULT "GLFW" DOCSTRING "Backend window and input implementation")
define_impl_list("RENDER" OPTIONS "OPENGL" DEFAULT "OPENGL" DOCSTRING "Backend render implementation")
define_impl_list("THREADING" OPTIONS "NONE" "EMSCRIPTEN" DEFAULT ${THREADING_DEFAULT} DOCSTRING "Backend threading implementation")
define_impl_list("RESOURCE" OPTIONS "FILESYSTEM" "EMSCRIPTEN" DEFAULT ${RESOURCE_DEFAULT} DOCSTRING "Resource-loading implementation")
define_impl_list("PLATFORM" OPTIONS "DEFAULT_" DEFAULT "DEFAULT_" DOCSTRING "Application setup and cleanup implementation")

# TODO: Remove this silly code. Just make the final defines in `define_impl_list` instead of waiting to collect them all here
# (Make sure to grab the real value from cache at the end and not the arg)
define_selected_impl_list()

#######################################
# Handle each chosen implementation
#######################################

add_subdirectory(window)
add_subdirectory(render)
add_subdirectory(threading)
add_subdirectory(resource)

if (IMPL_PLATFORM STREQUAL "DEFAULT_")
    target_sources(Glap PUBLIC default_platform.cpp)
else()
    message(SEND_ERROR "Missing platform implementation")
endif()