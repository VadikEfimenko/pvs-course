cmake_minimum_required(VERSION 3.17)
project(pong C)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(CMAKE_C_STANDARD 99)

add_executable(pong main.c sds/sds.h sds/sdsalloc.h sds/sds.c context/context.c context/context.h io_buf/io_buf.c io_buf/io_buf.h process/process.c process/process.h fsm/socket.c fsm/socket.h process/logger.c process/logger.h config/config.c config/config.h config/inih/ini.c config/inih/ini.h)

FIND_PACKAGE(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

INCLUDE_DIRECTORIES(
        src/include/
        ${GLIB_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${GLIB_LIBRARY_DIRS})

FILE(GLOB SRC src/*.c)

TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ${GLIB_LIBRARIES})

add_executable(pong_tests tests/test_main.c)