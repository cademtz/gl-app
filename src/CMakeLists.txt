
target_sources(Glap PUBLIC
    platform.cpp
    app.cpp
    main.cpp
    fnv1a.cpp
    font.cpp
    dialog.cpp
)

add_subdirectory(resources)
add_subdirectory(render)
add_subdirectory(input)
add_subdirectory(impl)