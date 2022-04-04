project(demo)

set(HEADER_FILES )
set(SOURCE_FILES main.cpp)

add_executable(${PROJECT_NAME} WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC engine)