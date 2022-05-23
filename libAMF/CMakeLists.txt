cmake_minimum_required(VERSION 3.1)

cmake_policy( SET CMP0042 NEW )
set( PROJECT AMF )

set( SOVERSION "0.1.0" )

cmake_policy( SET CMP0068 NEW )


set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules )
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

message( STATUS "TinyXML2 libs=${TINYXML2_LIBRARIES}" )

set( LIBRARIES ${TINYXML2_LIBRARIES}  )
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
