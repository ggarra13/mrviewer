#-*-cmake-*-
#
# Test for PostgreSQL libraries
#
# Once loaded this will define
#  POSTGRESQL_FOUND        - system has PostgreSQL
#  POSTGRESQL_INCLUDE_DIR  - include directory for PostgreSQL
#  POSTGRESQL_LIBRARY_DIR  - library directory for PostgreSQL
#  POSTGRESQL_LIBRARIES    - libraries you need to link to
#

SET(POSTGRESQL_FOUND "NO")

FIND_PATH( POSTGRESQL_LIBPQ_DIR libpq-fe.h
  $ENV{POSTGRESQL_ROOT}
  $ENV{POSTGRESQL_ROOT}/include
  $ENV{POSTGRESQL_ROOT}/interfaces/libpq
  /usr/include/postgresql
  )

FIND_PATH( POSTGRESQL_EXT_DIR postgres_ext.h
  $ENV{POSTGRESQL_ROOT}
  $ENV{POSTGRESQL_ROOT}/include
  $ENV{POSTGRESQL_ROOT}/src/include
  $ENV{POSTGRESQL_ROOT}/interfaces/libpq
  /usr/include/postgresql
  )

SET( POSTGRESQL_INCLUDE_DIR ${POSTGRESQL_LIBPQ_DIR} ${POSTGRESQL_EXT_DIR} )

FIND_LIBRARY( PQ       pq libpq
  PATHS 
  $ENV{POSTGRESQL_ROOT}/lib
  $ENV{POSTGRESQL_ROOT}/bin
  $ENV{POSTGRESQL_ROOT}/interfaces/libpq/Release
  /usr/local/lib
  /usr/lib
  DOC   "PostgreSQL library"
)

SET( POSTGRESQL_LIBRARIES ${PQ} )


IF (POSTGRESQL_INCLUDE_DIR)
  IF(POSTGRESQL_LIBRARIES)
    SET(POSTGRESQL_FOUND "YES")
    GET_FILENAME_COMPONENT(POSTGRESQL_LIBRARY_DIR ${PQ} PATH)
  ENDIF(POSTGRESQL_LIBRARIES)
ENDIF(POSTGRESQL_INCLUDE_DIR)

IF(NOT POSTGRESQL_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT POSTGRESQL_FIND_QUIETLY)
    IF(POSTGRESQL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "PostgreSQL required, please specify it's location with POSTGRESQL_ROOT")
    ELSE(POSTGRESQL_FIND_REQUIRED)
      MESSAGE(STATUS "PostgreSQL was not found.")
    ENDIF(POSTGRESQL_FIND_REQUIRED)
  ENDIF(NOT POSTGRESQL_FIND_QUIETLY)
ENDIF(NOT POSTGRESQL_FOUND)


#####

