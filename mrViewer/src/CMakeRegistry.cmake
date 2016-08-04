# Extra install commands for NSIS	

	set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		WriteRegStr HKCR 'mrViewer' '' 'mrViewer'
		WriteRegStr HKCR 'mrViewer\\\\shell' '' 'open'
		WriteRegStr HKCR 'mrViewer\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\mrViewer.exe \\\"%1\\\"'
	SectionEnd
	!addincludedir ${CMAKE_CURRENT_SOURCE_DIR}/nsis
	!include fileext.nsh
	Section \\\"empty\\\"
		"
		)


	set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
                DeleteRegKey HKCR 'mrViewer'
	SectionEnd
	!addincludedir ${CMAKE_CURRENT_SOURCE_DIR}/nsis
	!include fileext_uninstall.nsh
	Section \\\"un.empty\\\"
        "
        )
