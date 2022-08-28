cmake_minimum_required(VERSION 3.1)

cmake_policy( SET CMP0042 NEW )
set( PROJECT ACESclip )

set( SOVERSION "0.2.6" )

cmake_policy( SET CMP0068 NEW )

# This sets -fPIC on all libraries and code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list( INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/../modules)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ ISO Standard")
set(CMAKE_CXX_EXTENSIONS OFF)

set( CMAKE_MAKEFILE_VERBOSE 1 )

find_package( TinyXML2 REQUIRED )
find_package( Boost REQUIRED )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${TINYXML2_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  )

if(WIN32)
  add_definitions( -DACES_DLL -DACES_EXPORTS -D_WIN32_WINNT=0x0501 )
else(WIN32)
  if ( CMAKE_BUILD_TYPE EQUAL "Debug" )
    add_definitions( -DTHREADS_SANITIZER )
    add_compile_options( -fsanitize=thread )
  endif()
endif(WIN32)


add_library( ACESclip SHARED
  src/ACESclipWriter.cpp
  src/ACESclipReader.cpp
  )

set( LIBRARIES ${BOOST_LIBRARIES} ${TINYXML2_LIBRARIES} )
message( STATUS "LIBRARIES=${LIBRARIES}" )
target_link_libraries( ACESclip ${LIBRARIES} )

set_target_properties( ACESclip
  PROPERTIES
  VERSION ${SOVERSION}
  SOVERSION ${SOVERSION}
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")


set( ACESexecutables "" )

if( NOT DEFINED LIB_ACES_CLIP_ONLY )
  add_executable( ACESclipWriter examples/writer.cpp )
  target_link_libraries( ACESclipWriter ACESclip )

  add_executable( ACESclipReader examples/reader.cpp )
  target_link_libraries( ACESclipReader ACESclip )

  set( ACESexecutables ACESclipWriter ACESclipReader )

endif(NOT DEFINED LIB_ACES_CLIP_ONLY )

install( TARGETS ACESclip
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

if( NOT DEFINED LIB_ACES_CLIP_ONLY )
  install( FILES
    include/ACESclipReader.h
    include/ACESclipWriter.h
    include/ACESExport.h
    include/ACESTransform.h
    include/ACES_ASC_CDL.h
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
endif(NOT DEFINED LIB_ACES_CLIP_ONLY )
