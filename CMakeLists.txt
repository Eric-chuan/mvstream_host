cmake_minimum_required(VERSION 3.5)

project(mvstream_host LANGUAGES CXX)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I. -Wall -Werror -pedantic -fsanitize=address -g")


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_definitions(-DCMAKE)


file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(/usr/include)




add_executable(mvstream_host ${SOURCES})

TARGET_LINK_LIBRARIES(mvstream_host
        dl
        Threads::Threads
        -lcurl)