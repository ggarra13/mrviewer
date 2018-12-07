

cmake_minimum_required(VERSION 2.8)
set( PROJECT ACESclip )

set( SOVERSION "0.2.6" )

set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules )
find_package( TinyXML2 REQUIRED )
find_package( Boost REQUIRED )

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${TINYXML2_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  )

if(WIN32)
  add_definitions( -DNDEBUG -D_NDEBUG -DACES_DLL -DACES_EXPORTS -D_WIN32_WINNT=0x0501 )
  add_compile_options( -MD )
else(WIN32)
  if ( CMAKE_BUILD_TYPE EQUAL "Debug" )
    add_definitions( -DTHREADS_SANITIZER )
    add_compile_options( -fsanitize=thread )
  endif()
#  add_definitions( -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 )
endif(WIN32)


add_library( ACESclip SHARED 
  src/ACESclipWriter.cpp
  src/ACESclipReader.cpp 
  )

set( LIBRARIES ${TINYXML2_LIBRARIES} )

target_link_libraries( ACESclip ${LIBRARIES} )

set_target_properties( ACESclip
  PROPERTIES
  SOVERSION ${SOVERSION})


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
