set(_IMPL_CACHE_ENTRIES "" PARENT_SCOPE)

# define_impl_list(<backendName> OPTIONS <optionsList> DEFAULT <defaultValue> [DOCSTRING <docstring>])
#
# Defines an implementation setting with a list of available options, both in the CMake cache and the C/CXX compiler.
#
# The backendName will declare a cache entry and compiler definition.
#   (Example: "RENDER" becomes "IMPL_RENDER" in cache and "_IMPL_RENDER" in C/CXX)
# The optionsList items will become a series of enum-like compiler definitions in C/CXX.
#   (Example: "OPENGL" "VULKAN" becomes "_IMPL_RENDER_OPENGL=0" and "_IMPL_RENDER_VULKAN=1")
#
# backendName - Name for a backend being implemented
# optionsList - A list of backends to choose from
# defaultValue - A default value, must be contained in `optionsList`
# docstring - Optional, used as the cache entry's docstring
function(define_impl_list backendName)
    # Parse args (skip the first positional arg)
    set(oneValueKeywords DEFAULT DOCSTRING)
    set(multiValueKeywords OPTIONS)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "${oneValueKeywords}" "${multiValueKeywords}")

    message("ARG_DEFAULT: ${ARG_DEFAULT}")
    message("ARG_OPTIONS: ${ARG_OPTIONS}")
    message("ARG_DOCSTRING: ${ARG_DOCSTRING}")

    # Validate args
    if (NOT DEFINED ARG_DEFAULT)
        message(FATAL_ERROR "Missing DEFAULT arg")
    elseif (NOT DEFINED ARG_OPTIONS)
        message(FATAL_ERROR "Missing OPTIONS arg")
    elseif (NOT DEFINED ARG_DOCSTRING)
        message(FATAL_ERROR "Missing DOCSTRING arg")
    elseif (NOT "${ARG_DEFAULT}" IN_LIST ARG_OPTIONS)
        message(FATAL_ERROR "DEFAULT value \"${ARG_DEFAULT}\" is not one of the options in OPTIONS (${ARG_OPTIONS})")
    endif()

    set(ARG_NAME IMPL_${backendName})

    set(${ARG_NAME} ${ARG_DEFAULT} CACHE STRING ${ARG_DOCSTRING})
    set_property(CACHE ${ARG_NAME} PROPERTY STRINGS ${ARG_OPTIONS})

    set(index 0)
    foreach(option IN LISTS ARG_OPTIONS)
        message("  ${index}: ${option}")
        target_compile_definitions(Glap PUBLIC "_${ARG_NAME}_${option}=${index}")
        math(EXPR index "${index}+1")
    endforeach()

    # Append new entry to list using a hack to keep it in the calling script's scope
    set(NEW_LIST ${_IMPL_CACHE_ENTRIES})
    list(APPEND NEW_LIST ${ARG_NAME})
    set(_IMPL_CACHE_ENTRIES ${NEW_LIST} PARENT_SCOPE)
    message("All entries: ${NEW_LIST}")
endfunction()

# define_selected_impl_list()
#
# Creates C/CXX definitions indicating the currently values of every implementation added by `define_impl_list`.
#
# Example:
# An implementation setting, with a default value of "OPENGL", is created in the following code.
# ```cmake
# define_impl_list("RENDER" OPTIONS "OPENGL" "VULKAN" "DIRECTX" DEFAULT "OPENGL" DOCSTRING "Backend render implementation")
# ```
# 
# Many lines later, the default cached value is changed, by force, to "DIRECTX".
# Finally, `define_selected_impl_list` is called, creating the following compiler definition.
# ```
# _IMPL_RENDER=_IMPL_RENDER_DIRECTX
# ```
function(define_selected_impl_list)
    foreach (entry IN LISTS _IMPL_CACHE_ENTRIES)
        # Get the selected value
        get_property(entry_value CACHE ${entry} PROPERTY VALUE)

        # Define the selected implementation
        set(definition "_${entry}=_${entry}_${entry_value}")
        message("Comple definition: ${definition}")
        target_compile_definitions(Glap PUBLIC ${definition})
    endforeach()
endfunction()