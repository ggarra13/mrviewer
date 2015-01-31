

cmake_minimum_required(VERSION 2.8)

set( SOVERSION "0.1.0" )

set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules )
find_package( TinyXML2 REQUIRED )
find_package( Boost REQUIRED )

include_directories( 
  include
  ${TINYXML2_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  )

if(WIN32)
  add_definitions( -DACES_DLL -DACES_EXPORTS )
endif(WIN32)


add_library( ACESclip SHARED 
  lib/ACESclipWriter.cpp )

set( LIBRARIES ${TINYXML2_LIBRARIES} )

target_link_libraries( ACESclip ${LIBRARIES} )

set_target_properties( ACESclip
  PROPERTIES
  SOVERSION ${SOVERSION})

add_executable( ACESclipWriter examples/writer.cpp )
target_link_libraries( ACESclipWriter ACESclip )

install( TARGETS ACESclip ACESclipWriter
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib )

install( FILES 
  include/ACESclipWriter.h
  include/ACESExport.h
  include/ACESTransform.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
