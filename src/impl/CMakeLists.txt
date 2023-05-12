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

if (IMPL_WINDOW STREQUAL "GLFW")
    add_subdirectory(window/glfw)
else()
    message(SEND_ERROR "Missing window implementation")
endif()

if (IMPL_RENDER STREQUAL "OPENGL")
    add_subdirectory(render/opengl)
else()
    message(SEND_ERROR "Missing render implementation")
endif()

if (IMPL_THREADING STREQUAL "NONE")
    target_sources(Glap PUBLIC threading/no_threading.cpp)
elseif (IMPL_THREADING STREQUAL "EMSCRIPTEN")
    target_sources(Glap PUBLIC threading/em_threading.cpp)
else()
    message(SEND_ERROR "Missing threading implementation")
endif()

if (IMPL_RESOURCE STREQUAL "FILESYSTEM")
    add_subdirectory(resource/filesystem)
elseif (IMPL_RESOURCE STREQUAL "EMSCRIPTEN")
    add_subdirectory(resource/emscripten)
else()
    message(SEND_ERROR "Missing resource implementation")
endif()

if (IMPL_PLATFORM STREQUAL "DEFAULT_")
    target_sources(Glap PUBLIC default_platform.cpp)
else()
    message(SEND_ERROR "Missing platform implementation")
endif()