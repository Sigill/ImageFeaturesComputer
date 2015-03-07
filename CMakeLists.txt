cmake_minimum_required(VERSION 2.8)
project(FeaturesGraphComputer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

option(USE_LOG4CXX "Use log4cxx" ON)
mark_as_advanced(USE_LOG4CXX)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
# Should be shared libraries, and be compiled with the -fPIC flag

find_package(ITK_Haralick REQUIRED)
include_directories(${ITK_Haralick_INCLUDE_DIRS})

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    ON)
find_package(Boost COMPONENTS program_options system filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

if(USE_LOG4CXX)
	find_package(Log4Cxx REQUIRED)
	include_directories(${LOG4CXX_INCLUDE_DIR})
	add_definitions(-DUSE_LOG4CXX)
endif()

include(FindOpenMP)
if(OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#find_package(TULIP3 REQUIRED)
#include_directories(${TULIP_INCLUDE_DIR})

add_library(image_loader image_loader.cpp)

add_executable(features_computer features_computer.cpp cli_parser.cpp FeaturesComputerLoader.cpp)
target_link_libraries(features_computer image_loader ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES} ${ITK_LIBRARIES})

add_library(CoordinatesComputer SHARED CoordinatesComputer.cpp)
set_target_properties(CoordinatesComputer PROPERTIES COMPILE_FLAGS -fPIC)
target_link_libraries(CoordinatesComputer ${ITK_LIBRARIES})

add_library(HaralickComputer SHARED HaralickComputer.cpp)
set_target_properties(HaralickComputer PROPERTIES COMPILE_FLAGS -fPIC)
target_link_libraries(HaralickComputer ${ITK_LIBRARIES})

add_library(MeanValueComputer SHARED MeanValueComputer.cpp)
set_target_properties(MeanValueComputer PROPERTIES COMPILE_FLAGS -fPIC)
target_link_libraries(MeanValueComputer ${ITK_LIBRARIES})

add_executable(channel_cutter channel_cutter.cpp)
target_link_libraries(channel_cutter image_loader ${ITK_LIBRARIES} ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES})

if(USE_LOG4CXX)
	target_link_libraries(features_computer image_loader ${LOG4CXX_LIBRARIES})
	target_link_libraries(channel_cutter image_loader ${LOG4CXX_LIBRARIES})
endif()

CONFIGURE_FILE(features_computer.sh "${PROJECT_BINARY_DIR}/features_computer.sh" COPYONLY)

