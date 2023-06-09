
target_sources(Glap PUBLIC
    demostuff.cpp
    platform.cpp
    app.cpp
    main.cpp
    fnv1a.cpp
    font.cpp
)

add_subdirectory(resources)
add_subdirectory(controls)
add_subdirectory(render)
add_subdirectory(input)
add_subdirectory(impl)