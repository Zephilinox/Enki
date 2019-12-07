cmake_minimum_required (VERSION 3.15)

message(STATUS "Building Docs")
find_package(DOXYGEN REQUIRED)

if (DOXYGEN_FOUND)
	set(DOXYGEN_IN ${EnkiEngine_SOURCE_DIR}/docs/Doxyfile.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile.out)
	set(DOXYGEN_INPUT "../../../src/")
	set(DOXYGEN_BUILD_DIRECTORY "./doxygen/")
	
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	
	add_custom_target(docs_doxygen ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen
		COMMENT "Generating documentation with Doxygen"
		VERBATIM)
	
	find_package(SPHINX)
	if (SPHINX_FOUND) 
		if(NOT DEFINED SPHINX_THEME)
			set(SPHINX_THEME bootstrap)
		endif()
		message(STATUS "SPHINX_THEME=${SPHINX_THEME}")
		 
		if(NOT DEFINED SPHINX_THEME_DIR)
			set(SPHINX_THEME_DIR "sphinx_bootstrap_theme.get_html_theme_path()")
		endif()
		message(STATUS "SPHINX_THEME_DIR=${SPHINX_THEME_DIR}")
		
		if(NOT DEFINED SPHINX_BOOTSWATCH_THEME)
			set(SPHINX_BOOTSWATCH_THEME cosmo)
		endif()
		message(STATUS "SPHINX_BOOTSWATCH_THEME=${SPHINX_BOOTSWATCH_THEME}")
		
		set(SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
		set(SPHINX_BINARY_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
		set(SPHINX_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/_doctrees)
		set(SPHINX_HTML_DIR ${CMAKE_CURRENT_BINARY_DIR}/html)
		
		configure_file(
			${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in
			${SPHINX_BINARY_BUILD_DIR}/conf.py
			@ONLY)
		
		
		add_custom_target(docs_sphinx ALL
						  COMMAND ${CMAKE_COMMAND} -E make_directory api
						  COMMAND
						  ${SPHINX_EXECUTABLE} -b html
						  -c ${SPHINX_BINARY_BUILD_DIR}
						  -d ${SPHINX_CACHE_DIR}
						  ${SPHINX_SOURCE_DIR}
						  ${SPHINX_HTML_DIR}
						  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
						  COMMENT "Generating documentation with Sphinx")
	else()
		message(WARNING "Sphinx could not be found when building the docs.")
	endif()
		
	message(STATUS "Docs will be generated when building the project")
endif()