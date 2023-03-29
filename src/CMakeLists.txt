# Provide platform-specific implementations of our own classes

target_sources(Glap PUBLIC
    platform.cpp
    app.cpp
    main.cpp
)

add_subdirectory(impl)
add_subdirectory(resources)
add_subdirectory(render)