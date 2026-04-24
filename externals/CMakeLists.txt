# glad
add_library(glad glad/src/gl.c glad/include/glad/gl.h)
target_include_directories(glad SYSTEM PUBLIC glad/include)
set_target_properties(glad PROPERTIES FOLDER "externals")

# glm
add_subdirectory(glm)
set_target_properties(glm PROPERTIES FOLDER "externals")
#include_directories(${CMAKE_SOURCE_DIR}/externals/glm)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
set_target_properties(glfw PROPERTIES FOLDER "externals")
set_target_properties(update_mappings PROPERTIES FOLDER "externals")


# imgui
add_library(imgui imgui/imgui.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp imgui/imgui_draw.cpp imgui/imgui_demo.cpp)
target_include_directories(imgui SYSTEM PUBLIC imgui)
set_target_properties(imgui PROPERTIES FOLDER "externals")

# imgui backend
add_library(imgui_glfw_opengl3 imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp)
target_compile_definitions(imgui_glfw_opengl3 PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui_glfw_opengl3 imgui)
target_link_libraries(imgui_glfw_opengl3 glad)
target_link_libraries(imgui_glfw_opengl3 glfw)
target_include_directories(imgui_glfw_opengl3 SYSTEM PUBLIC imgui/backends)
set_target_properties(imgui_glfw_opengl3 PROPERTIES FOLDER "externals")

# assimp
SET(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
SET(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp)
set_target_properties(assimp PROPERTIES FOLDER "externals")
set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "externals")
set_target_properties(zlibstatic PROPERTIES FOLDER "externals")

# stb
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE stb)
set_target_properties(stb PROPERTIES FOLDER "externals")

# GLSL-Shader-Includes
add_library(glsl-shader-includes INTERFACE)
target_include_directories(glsl-shader-includes SYSTEM INTERFACE GLSL-Shader-Includes)
set_target_properties(glsl-shader-includes PROPERTIES FOLDER "externals")

# spdlog
#SET(SPDLOG_MASTER_PROJECT ON CACHE BOOL "" FORCE)
# SET(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
# add_subdirectory(spdlog)
# set_target_properties(spdlog PROPERTIES FOLDER "externals")

# mikktspace
add_library(mikktspace STATIC mikktspace/mikktspace.c)
target_include_directories(mikktspace PUBLIC mikktspace)
set_target_properties(mikktspace PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    FOLDER "externals"
)
