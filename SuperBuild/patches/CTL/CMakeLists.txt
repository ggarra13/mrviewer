include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/lib/IlmCtl" "${PROJECT_SOURCE_DIR}/lib/IlmCtlMath" )

find_package( OpenEXR REQUIRED )


if( WIN32 )
   FILE( COPY ${CMAKE_INSTALL_PREFIX}/bin/Imath-3_2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
endif()

add_executable( halfExpLogTable halfExpLogTable.cpp )
target_link_libraries( halfExpLogTable ${IlmBase_LDFLAGS_OTHER} )
target_link_libraries( halfExpLogTable ${OpenEXR_LIBRARIES} )


add_custom_command( OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/halfExpLogTable.h"
		    COMMAND halfExpLogTable "${CMAKE_CURRENT_BINARY_DIR}/halfExpLogTable.h" "${CMAKE_CURRENT_BINARY_DIR}/halfExpLog.h"
		    DEPENDS halfExpLogTable )

include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )

if(WIN32)
  add_definitions( -DOPENEXR_DLL )
endif(WIN32)

add_library( IlmCtlSimd ${DO_SHARED}
	CtlSimdAddr.cpp
	CtlSimdFunctionCall.cpp
	CtlSimdHalfExpLog.cpp
	CtlSimdInst.cpp
	CtlSimdInterpreter.cpp
	CtlSimdLContext.cpp
	CtlSimdModule.cpp
	CtlSimdReg.cpp
	CtlSimdStdLibAssert.cpp
	CtlSimdStdLibColorSpace.cpp
	CtlSimdStdLibInterpolator.cpp
	CtlSimdStdLibLimits.cpp
	CtlSimdStdLibLookupTable.cpp
	CtlSimdStdLibMath.cpp
	CtlSimdStdLibPrint.cpp
	CtlSimdStdLibrary.cpp
	CtlSimdStdTypes.cpp
	CtlSimdSyntaxTree.cpp
	CtlSimdType.cpp
	CtlSimdXContext.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/halfExpLogTable.h"
)

target_link_libraries( IlmCtlSimd ${IlmBase_LDFLAGS_OTHER} )
target_link_libraries( IlmCtlSimd IlmCtlMath IlmCtl ${OPENEXR_LIBRARIES} )

set_target_properties( IlmCtlSimd PROPERTIES
  VERSION ${CTL_VERSION}
  SOVERSION ${CTL_VERSION}
)

install( FILES CtlSimdInterpreter.h DESTINATION include/CTL )

install( TARGETS IlmCtlSimd DESTINATION lib )
