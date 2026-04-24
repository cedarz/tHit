cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE source_list src/*.h src/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_list})
add_compile_options(/utf-8)

add_executable(model-viewer ${source_list})
target_include_directories(model-viewer PRIVATE src
    ${CMAKE_SOURCE_DIR}/externals/glm
    ${CMAKE_SOURCE_DIR}/externals/stb
    ${CMAKE_SOURCE_DIR}/externals/spdlog/include
    ${CMAKE_SOURCE_DIR}/externals/filesystem/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(model-viewer PRIVATE 
    glfw 
    imgui
    imgui_glfw_opengl3
    mikktspace
    assimp::assimp
)

# set cmake source dir macro 
target_compile_definitions(model-viewer PRIVATE
    CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    -DASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

add_custom_command(TARGET model-viewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:assimp>
        $<TARGET_FILE_DIR:model-viewer>)