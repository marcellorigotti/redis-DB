cmake_minimum_required(VERSION 3.10)
project(RedisDB)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_subdirectory(src/dataStructures)
add_executable(server ./src/server.cpp)
add_executable(client ./src/client.cpp)
add_executable(serverEventLoop ./src/serverEventLoop.cpp)
add_executable(testAvl ./src/dataStructures/testAvl.cpp)
target_include_directories(serverEventLoop PUBLIC
                          "$build"
                          "$src/dataStructures"
                          )
target_link_libraries(serverEventLoop PUBLIC HashTable)
target_link_libraries(serverEventLoop PUBLIC AvlTree)
target_link_libraries(testAvl PUBLIC AvlTree)


