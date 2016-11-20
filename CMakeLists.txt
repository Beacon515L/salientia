cmake_minimum_required(VERSION 2.6)
project(salientia)
#add_library(ly ly.cpp ly.h)
set( SOURCES ly.h ly.cpp recurse.h recurse.cpp main.cpp )
#set(CMAKE_C_COMPILER "cc")
set(CMAKE_CXX_COMPILER "g++")



#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 
#find_package(Boost 1.45.0 REQUIRED COMPONENTS filesystem ) 
   add_executable(salientia ${SOURCES} )
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC salientia "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(salientia "${CMAKE_THREAD_LIBS_INIT}")
endif()

#if(Boost_FOUND)

#    include_directories(${Boost_INCLUDE_DIRS}) 

#    target_link_libraries(salientia ${Boost_LIBRARIES})
    # install(TARGETS salientia RUNTIME DESTINATION bin)
#endif()

