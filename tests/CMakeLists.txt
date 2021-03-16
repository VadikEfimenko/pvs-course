cmake_minimum_required(VERSION 3.17)
project(tests C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall")

add_executable(tests test_main.c test_handlers.c test_socket.c ../sds/sds.h ../sds/sds.c ../context/context.c ../context/context.h ../fsm/socket.c ../fsm/socket.h ../process/process.c ../process/process.h ../process/logger.c ../process/logger.h ../io_buf/io_buf.c ../io_buf/io_buf.h)

set_target_properties(tests PROPERTIES LINK_FLAGS -lcunit)

FIND_PACKAGE(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

INCLUDE_DIRECTORIES(
        src/include/
        ${GLIB_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${GLIB_LIBRARY_DIRS})

FILE(GLOB SRC src/*.c)

TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ${GLIB_LIBRARIES} cunit)