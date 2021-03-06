project(ChatServer)
cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)
aux_source_directory(src SRC_LIST)
file(GLOB SRC . *.cpp)
file(GLOB HEADERS "include/*.h")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WEBSOCKETPP_CPP11_STL_")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -lpqxx -lpq") 

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
elseif(CMAKE_VERSION VERSION_GREATER "3.1")
	# Cause of my arch linux could not detect one of the c++11 variants so i added another set
	# Only available in cmake 3.1 or greater
	set (CMAKE_CXX_STANDARD 11)	
else ()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost 1.59.0 REQUIRED COMPONENTS system thread regex)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ELSE()
  INCLUDE_DIRECTORIES(lib/boost/)
  LINK_DIRECTORIES(lib/boost/lib/)
ENDIF(Boost_FOUND)


FIND_PACKAGE(OpenSSL REQUIRED)
IF(OPENSSL_FOUND)
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  LINK_DIRECTORIES(${OPENSSL_LIBRARIES})
ENDIF(OPENSSL_FOUND)

INCLUDE_DIRECTORIES(include/)
INCLUDE_DIRECTORIES(lib/websocketpp/)
INCLUDE_DIRECTORIES(lib/rapidjson/include/)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS} main.cpp)

target_link_libraries( ${PROJECT_NAME} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
