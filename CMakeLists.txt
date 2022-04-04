cmake_minimum_required(VERSION 3.10)

project(dx_roadmap VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

file(REMOVE_RECURSE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/demo/resources/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/demo/resources/config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/ COPYONLY)

option(BUILD_DEMO "Build demo" ON)

add_subdirectory(engine)

if(BUILD_DEMO)
    add_subdirectory(demo)
endif()