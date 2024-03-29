cmake_minimum_required(VERSION 3.1)

cmake_policy( SET CMP0042 NEW )
set( PROJECT AMF )

set( SOVERSION "0.1.0" )

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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/md5
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SHA1
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SHA256
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${TINYXML2_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  )

add_definitions(  -DAMF_EXPORTS )
if(WIN32)
  add_definitions( -DAMF_DLL -D_WIN32_WINNT=0x0501 )
endif()


add_library( AMF SHARED
  src/SHA256/SHA256.cpp
  src/AMFWriter.cpp
  src/AMFReader.cpp
  )


set( LIBRARIES ${BOOST_LIBRARIES} ${TINYXML2_LIBRARIES}  )
target_link_libraries( AMF ${LIBRARIES} )

set_target_properties( AMF
  PROPERTIES
  VERSION ${SOVERSION}
  SOVERSION ${SOVERSION}
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")


set( AMFexecutables "" )

if( NOT DEFINED LIB_AMF_ONLY )
  add_executable( AMFWriter examples/writer.cpp )
  target_link_libraries( AMFWriter AMF )

  add_executable( AMFReader examples/reader.cpp )
  target_link_libraries( AMFReader AMF  )

  set( AMFexecutables AMFWriter AMFReader )

endif()

install( TARGETS AMF
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

if( NOT DEFINED LIB_AMF_ONLY )
  install( FILES
    src/AMFReader.h
    src/AMFWriter.h
    src/AMFBase.h
    src/AMFExport.h
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
endif()
