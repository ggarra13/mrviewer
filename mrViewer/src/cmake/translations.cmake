#
# This file contains the .pot / .po / .mo language translations
#
#

set( _absPotFile "${CMAKE_CURRENT_SOURCE_DIR}/po/messages.pot" )

add_custom_command( OUTPUT "${_absPotFile}"
  COMMAND xgettext
  ARGS --package-name=mrViewer --package-version="$VERSION" --copyright-holder="Film Aura, LLC" --msgid-bugs-address=ggarra13@gmail.com -d mrViewer -s -c++ -k_ ${SOURCES} -o po/messages.pot
  DEPENDS mrViewer
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set( LANGUAGES "ar" "cs" "de" "en" "es" "fr" "gr" "it" "ja"
	       "ko" "nl" "pl" "pt" "ro" "ru" "sv" "tr" "zh" )

set( output_files "${_absPotFile}" )

foreach( lang ${LANGUAGES} )

  set( _moDir "${CMAKE_SOURCE_DIR}/share/locale/${lang}/LC_MESSAGES/" )
  set( _moFile "${_moDir}/mrViewer${SHORTVERSION}.mo" )

  set( output_files ${output_files} ${_moFile} )

  file( REMOVE_RECURSE "${_moDir}" ) # Remove dir to remove old .mo files
  file( MAKE_DIRECTORY "${_moDir}" ) # Recreate dir to place new .mo file

  set( _absFile "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}.po" )

  add_custom_command( OUTPUT "${_moFile}"
    COMMAND msgmerge --quiet --update --backup=none
    "${_absFile}" "${_absPotFile}"
    COMMAND msgfmt -v "${_absFile}" -o "${_moFile}"
    DEPENDS ${_absFile} ${_absPotFile}
  )

endforeach( lang )

ADD_CUSTOM_TARGET(
  "translations" ALL
  DEPENDS ${output_files} ${PROJECT_NAME}
  )
