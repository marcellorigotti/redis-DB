cmake_minimum_required(VERSION 3.10)
project(RedisDB)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_subdirectory(src/dataStructures)
add_executable(server ./src/server.cpp)
add_executable(client ./src/client.cpp)
add_executable(serverEventLoop ./src/serverEventLoop.cpp)
add_executable(avlTreeTest ./src/dataStructures/avlTreeTest.cpp)
add_executable(sortedSetTest ./src/dataStructures/sortedSetTest.cpp)
target_include_directories(serverEventLoop PUBLIC
                          "$build"
                          "$src/dataStructures"
                          )
target_link_libraries(serverEventLoop PUBLIC HashTable)
target_link_libraries(serverEventLoop PUBLIC AvlTree)
target_link_libraries(serverEventLoop PUBLIC SortedSet)
target_link_libraries(avlTreeTest PUBLIC AvlTree)
target_link_libraries(sortedSetTest PUBLIC SortedSet)
target_link_libraries(sortedSetTest PUBLIC AvlTree)
target_link_libraries(sortedSetTest PUBLIC HashTable)





