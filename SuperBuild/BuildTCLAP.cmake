set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/TCLAP/CMakeLists.txt ${CMAKE_BINARY_DIR}/TCLAP-prefix/src/TCLAP/ )

set( install_cmd ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include/tclap && cp -r ${CMAKE_BINARY_DIR}/TCLAP-prefix/src/TCLAP/include/tclap/ ${CMAKE_INSTALL_PREFIX}/include/ )

  ExternalProject_Add(
    TCLAP
    GIT_REPOSITORY "https://github.com/mirror/tclap.git"
    GIT_PROGRESS 1
    PATCH_COMMAND ${patch_command}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_CXX_FLAGS=${cxx_flags}
    ${MRV_EXTERNAL_ARGS}
    INSTALL_COMMAND ${install_cmd}
    )

  set( TCLAP "TCLAP" )
