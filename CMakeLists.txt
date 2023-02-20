cmake_minimum_required(VERSION 3.16)
project(Glap)

find_package(OpenGL REQUIRED)

if (EMSCRIPTEN)
	file(GLOB_RECURSE GLAP_SOURCE_LIST "src/*.cpp")
	add_executable(Glap ${GLAP_SOURCE_LIST})

	set(CMAKE_EXE_LINKER_FLAGS
		"-s USE_GLFW=3 -s FETCH --proxy-to-worker"
		# -s USE_WEBGL2=1 # I think this version is already default or higher
	)
	include_directories(deps/glfw3/include)

else()
	# Find the appropriate GLFW3 library ourselves
	file(GLOB_RECURSE GLAP_SOURCE_LIST "src/*.cpp")
	add_executable(Glap ${GLAP_SOURCE_LIST} deps/glad/src/gl.c)

	target_link_libraries(Glap OpenGL::GL)

	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	
	add_subdirectory(deps/glfw3)
	add_subdirectory(deps/glad)
	target_link_libraries(Glap glad glfw)
	include_directories(deps/glad/include)
endif()

# Link all platform-independent libraries
add_subdirectory(deps/glm)
target_link_libraries(Glap glm)

include_directories(
	#deps/glfw3/include
	deps/stb
	src
)