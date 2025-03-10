cmake_minimum_required(VERSION 3.16)
project(Glap)

find_package(OpenGL REQUIRED)
add_executable(Glap)

target_compile_features(Glap PUBLIC cxx_std_20)
set(CXX_STANDARD_REQUIRED ON)

# Make VSCode useable under code-server 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC) # If we're using Microsoft Compiler (tm)
    add_compile_options(/MP) # Enable multi-threaded compiling
    add_link_options(/DEBUG:FULL)
endif()

if (EMSCRIPTEN)
	
	#set(CMAKE_CXX_FLAGS "-pthread")
	set(CMAKE_CXX_FLAGS "-g -gsource-map --source-map-base https://epic-server.tk/code/proxy/8003/build/")
	set(CMAKE_EXE_LINKER_FLAGS
		"-s USE_GLFW=3 -s FETCH -s ASYNCIFY -s USE_WEBGL2=1 --memoryprofiler --cpuprofiler" # I think this version is already default or higher
	)

	# Emscripten already provides OpenGL-related libraries, except the glfw headers apparently
	include_directories(deps/glfw3/include)

	file(COPY shell.html resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

else()
	# Provide the appropriate OpenGL-related libraries ourselves
	target_sources(Glap PUBLIC deps/glad/src/gl.c)

	target_link_libraries(Glap OpenGL::GL)

	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	
	add_subdirectory(deps/glfw3)
	add_subdirectory(deps/glad)
	target_link_libraries(Glap glad glfw)
	include_directories(deps/glad/include)
endif()

add_subdirectory(src)

# Link all platform-independent libraries
add_subdirectory(deps/glm)

target_link_libraries(Glap glm)

include_directories(
	#deps/glfw3/include
	deps/stb
	src
)