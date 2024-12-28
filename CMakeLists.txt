cmake_minimum_required(VERSION 3.30)
project(whatami)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add flags for 32-bit architecture
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")

# Static linking options
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")

# Add executable
add_executable(whatami main.cpp
        windows_edition.h
        windows_edition.cpp)
