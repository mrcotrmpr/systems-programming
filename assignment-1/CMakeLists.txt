cmake_minimum_required(VERSION 3.20)

project(assignment-1)

set(CMAKE_CXX_STANDARD 20)

if (WIN32 OR WIN64)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

include_directories(lib)
add_subdirectory(lib)

add_custom_target(copy_data COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)

add_executable(${PROJECT_NAME} main.cpp)
add_dependencies(${PROJECT_NAME} copy_data)

target_link_libraries(${PROJECT_NAME} PRIVATE lib-static lib-shared)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:lib-shared>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
