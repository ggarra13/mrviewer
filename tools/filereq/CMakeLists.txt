
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


ADD_EXECUTABLE( filereq main.cpp )
TARGET_LINK_LIBRARIES( filereq ${LIBRARIES} )
