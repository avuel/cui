cmake_minimum_required(VERSION 4.0.0)
project(cui LANGUAGES C)

cmake_path(SET shader_vertex_source NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex.hlsl")
cmake_path(SET shader_vertex_output NORMALIZE "${CMAKE_CURRENT_BINARY_DIR}/shaders/vertex.dxil")
cmake_path(SET shader_pixel_source NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/pixel.hlsl")
cmake_path(SET shader_pixel_output NORMALIZE "${CMAKE_CURRENT_BINARY_DIR}/shaders/pixel.dxil")

add_custom_command(
        OUTPUT ${shader_vertex}
        COMMAND dxc.exe -T vs_6_0 -E vs_main -Fo ${shader_vertex} ${shader_source}
        DEPENDS ${shader_source}
        COMMENT "compiling vertex shader: ${shader_source} -> ${shader_vertex}"
        VERBATIM
)

add_custom_command(
        OUTPUT ${shader_pixel}
        COMMAND dxc.exe -T ps_6_0 -E ps_main -Fo ${shader_pixel} ${shader_source}
        DEPENDS ${shader_source}
        COMMENT "compiling pixel shader: ${shader_source} -> ${shader_pixel}"
        VERBATIM
)

set(shaders_compiled ${shader_vertex} ${shader_pixel})
add_custom_target(cui_shaders DEPENDS ${shaders_compiled})

add_executable(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} cui_shaders)

target_sources(${PROJECT_NAME}
        PRIVATE
            main.c
)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
            C_STANDARD 99
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS ON
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            nuklear_sdl::nuklear_sdl
)
