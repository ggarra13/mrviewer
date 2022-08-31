if (POLICY CMP0068)
  cmake_policy( SET CMP0068 NEW )
endif()

if(POLICY CMP0074)
  # enable find_package(<Package>) to use <Package>_ROOT as a hint
  cmake_policy(SET CMP0074 NEW)
endif()

if (POLICY CMP0092)
  cmake_policy( SET CMP0092 NEW )
endif()

# Use extraction timestamps
if( POLICY CMP0135 )
  cmake_policy( SET CMP0135 NEW )
endif()

# cmake_policy(SET CMP0091 NEW)  # new microsoft behavior
