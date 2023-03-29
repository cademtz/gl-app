cmake_minimum_required(VERSION 3.16)
project(Glap)

find_package(OpenGL REQUIRED)
add_executable(Glap)

target_compile_features(Glap PUBLIC cxx_std_17)
set(CXX_STANDARD_REQUIRED ON)

if (EMSCRIPTEN)
	
	#set(CMAKE_CXX_FLAGS "-pthread")
	set(CMAKE_EXE_LINKER_FLAGS
		"-s USE_GLFW=3 -s FETCH -s USE_WEBGL2=1" # I think this version is already default or higher
	)

	# Emscripten already provides OpenGL-related libraries, except the glfw headers apparently
	include_directories(deps/glfw3/include)

	file(COPY shell.html shell.js DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

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