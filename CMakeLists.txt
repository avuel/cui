cmake_minimum_required(VERSION 4.0.0)
project(cui LANGUAGES C)

add_executable(${PROJECT_NAME} main.c)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
            C_STANDARD 99
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS OFF
)
