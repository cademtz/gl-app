if (IMPL_WINDOW STREQUAL "GLFW")
    add_subdirectory(glfw)
else()
    message(SEND_ERROR "Missing window implementation")
endif()