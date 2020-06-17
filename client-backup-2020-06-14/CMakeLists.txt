cmake_minimum_required(VERSION 3.10)

project(lillietech-3d-scanner-client)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost
  1.71                              
  REQUIRED                          
  COMPONENTS program_options  
  )                                 
set(CMAKE_CXX_VERSION 17)
add_executable( 3dscan
  main.cpp
)

target_link_libraries(3dscan boost_program_options.a)
