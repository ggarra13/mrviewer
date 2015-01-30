

cmake_minimum_required(VERSION 2.8)

set( SOVERSION "0.1.0" )

set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules )
find_package( TinyXML2 REQUIRED )
find_package( Boost REQUIRED )

include_directories( 
  include
  ${Boost_INCLUDE_DIR}
  )

add_library( ACESclip SHARED 
  lib/ACESclipWriter.cpp )

set( LIBRARIES tinyxml2 )

target_link_libraries( ACESclip ${LIBRARIES} )

set_target_properties( ACESclip
  PROPERTIES
  SOVERSION ${SOVERSION})

add_executable( ACESclipWriter examples/writer.cpp )
target_link_libraries( ACESclipWriter ACESclip )

install( TARGETS ACESclip ACESclipWriter
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib )

install( FILES include/ACESclipWriter.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
