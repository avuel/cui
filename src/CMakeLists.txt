cmake_minimum_required(VERSION 4.0.0)
project(cui LANGUAGES C)


list(APPEND shader_sources "shader/world/hlsl/vertex.hlsl")
list(APPEND shader_sources "shader/world/hlsl/pixel.hlsl")
list(APPEND shader_sources "shader/gui/hlsl/vertex.hlsl")
list(APPEND shader_sources "shader/gui/hlsl/pixel.hlsl")

foreach(shader IN LISTS shader_sources)
    cmake_path(SET shader_source NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/${shader}")
    cmake_path(SET shader_embed  NORMALIZE "${CMAKE_CURRENT_BINARY_DIR}/${shader}.c")

    cmake_path(GET shader_source             PARENT_PATH shader_language_directory)
    cmake_path(GET shader_language_directory PARENT_PATH shader_pipeline_directory)

    cmake_path(GET shader_source             STEM     shader_stage)
    cmake_path(GET shader_language_directory FILENAME shader_language)
    cmake_path(GET shader_pipeline_directory FILENAME shader_pipeline)

    set(shader_variable_name shader_${shader_pipeline}_${shader_stage}_${shader_language})

#    message(STATUS "shader source: ${shader_source}")
#    message(STATUS "shader stage: ${shader_stage}")
#    message(STATUS "shader language: ${shader_language}")
#    message(STATUS "shader pipeline: ${shader_pipeline}")
    message(STATUS "shader source: ${shader_source}")
    message(STATUS "shader embed: ${shader_embed}")
    message(STATUS "shader variable name: ${shader_variable_name}")

    # call xxd on vertex shader to embed into .c file
    add_custom_command(
            OUTPUT ${shader_embed}
            # TODO: windows xxd does not have -t option for null termination...
            COMMAND xxd -i -n ${shader_variable_name} ${shader_source} ${shader_embed}
            DEPENDS ${shader_source}
            COMMENT "embedding shader ${shader_source} -> ${shader_embed}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
    )

    list(APPEND shader_embeds ${shader_embed})
endforeach()

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/main.c
            ${CMAKE_CURRENT_SOURCE_DIR}/camera.c
            ${CMAKE_CURRENT_SOURCE_DIR}/gui.c

            # shader
            ${CMAKE_CURRENT_SOURCE_DIR}/shader.c
            # shader programs
            ${shader_embeds}
)

# expose nuklear header
target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET nuklear_HEADERS
        TYPE HEADERS
        BASE_DIRS ${nuklear_SOURCE_DIR}
        FILES ${nuklear_SOURCE_DIR}/nuklear.h
)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
            C_STANDARD 99
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS ON
            POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            cglm_headers
            SDL3::SDL3
            SDL3_shadercross::SDL3_shadercross
)
