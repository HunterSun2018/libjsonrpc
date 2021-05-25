cmake_minimum_required(VERSION 3.10.0)

project(json-prc-client VERSION 1.0)

# specify the c++ standard 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")       # -Werror
# set(CMAKE_C_COMPILER "gcc-10")
# set(CMAKE_CXX_COMPILER "g++-10")
# set(CMAKE_LINK_FLAG -static-libgcc)

include_directories($ENV{HOME}/boost_1_76_0)
add_definitions(-DBOOST_ASIO_HAS_CO_AWAIT -DBOOST_ASIO_HAS_STD_COROUTINE -fcoroutines)

aux_source_directory(src SRC)

add_executable(json-rpc-client ${SRC})

# set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

# target_link_options()

target_link_libraries(json-rpc-client ssl crypto pthread)

install(TARGETS json-rpc-client
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

#add_subdirectory(test)
enable_testing()
add_test(NAME mytest COMMAND Test)

# mkdir build/release
# cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=. -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc"