cmake_minimum_required (VERSION 3.1)
project (mysql-proxy)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FILE(GLOB Sources src/*.cpp src/*.h)
include_directories (3rd-party/asio-1.12.0/include)
add_definitions(-DASIO_STANDALONE)
IF (WIN32)
  add_definitions(-D_WIN32_WINNT=0x0501)
ENDIF()

add_executable(mysql-proxy ${Sources})