cmake_minimum_required(VERSION 3.10)
project(task_for_acos)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Windows нужно линковать Winsock
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lws2_32")
endif()

# Клиент
add_executable(client client/client.cpp)
if(WIN32)
    target_link_libraries(client ws2_32)
endif()

# Сервер
add_executable(server server/server.cpp)
if(WIN32)
    target_link_libraries(server ws2_32)
endif()