cmake_minimum_required(VERSION 2.8)
project(FeaturesGraphComputer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(ITK_Haralick REQUIRED)
include_directories(${ITK_Haralick_INCLUDE_DIRS})

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    ON)
find_package(Boost COMPONENTS program_options system filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Log4Cxx REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIR})

include(FindOpenMP)
if(OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#find_package(TULIP3 REQUIRED)
#include_directories(${TULIP_INCLUDE_DIR})

add_library(image_loader image_loader.cpp)

add_executable(coordinates generate_coordinates_image.cpp)
target_link_libraries(coordinates image_loader ${ITK_LIBRARIES} ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES})

add_executable(haralick generate_haralick_image.cpp)
target_link_libraries(haralick image_loader ${ITK_LIBRARIES} ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES})

