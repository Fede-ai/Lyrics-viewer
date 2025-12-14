cmake_minimum_required(VERSION 3.10)
project(Lyrics-viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Restricted build types" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_directory_properties(PROPERTIES VS_SOLUTION_ITEMS "LICENSE.md;README.md;.gitignore;TO_DO.md")
source_group("Documentation" FILES LICENSE.md README.md .gitignore TO_DO.md)

set(SOURCE
	src/button.hpp 
	src/curlwrapper.cpp src/curlwrapper.hpp 
	src/json.hpp 
	src/main.cpp 
	src/overlay.cpp src/overlay.hpp
)

add_executable(Lyrics-viewer WIN32 ${SOURCE})
target_sources(Lyrics-viewer PRIVATE src/app.rc)

set_target_properties(Lyrics-viewer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Lyrics-viewer>"
)
target_link_options(Lyrics-viewer PRIVATE
    $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>
)

set(PATH_SFML "C:/SFML-3.0.0" CACHE STRING "Path to the SFML library")
set(PATH_LIBCURL "C:/curl-8.12.1" CACHE STRING "Path to the libcurl library")

target_include_directories(Lyrics-viewer PRIVATE ${PATH_SFML}/include)
target_include_directories(Lyrics-viewer PRIVATE ${PATH_LIBCURL}/include)

target_link_directories(Lyrics-viewer PRIVATE
	${PATH_SFML}/lib
	${PATH_LIBCURL}/lib
)

target_link_libraries(Lyrics-viewer PRIVATE
	$<$<CONFIG:Release>:sfml-system.lib sfml-window.lib sfml-graphics.lib sfml-main-s.lib>
	$<$<CONFIG:Debug>:sfml-system-d.lib sfml-window-d.lib sfml-graphics-d.lib sfml-main-s-d.lib>
	libcurl.lib
)

# rename 'libcurl.dll.a' to 'libcurl.lib'
add_custom_command(TARGET Lyrics-viewer PRE_LINK
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PATH_LIBCURL}/lib/libcurl.dll.a ${PATH_LIBCURL}/lib/libcurl.lib
    COMMENT "Renaming 'libcurl.dll.a' to 'libcurl.lib'..."
)

# copy necessary dll files and resource folders
add_custom_command(TARGET Lyrics-viewer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different        	
	$<$<CONFIG:Release>:${PATH_SFML}/bin/sfml-system-3.dll>
	$<$<CONFIG:Debug>:${PATH_SFML}/bin/sfml-system-d-3.dll>
	$<$<CONFIG:Release>:${PATH_SFML}/bin/sfml-window-3.dll>
	$<$<CONFIG:Debug>:${PATH_SFML}/bin/sfml-window-d-3.dll>
	$<$<CONFIG:Release>:${PATH_SFML}/bin/sfml-graphics-3.dll>
	$<$<CONFIG:Debug>:${PATH_SFML}/bin/sfml-graphics-d-3.dll>
	"${PATH_LIBCURL}/bin/libcurl-x64.dll"
	$<TARGET_FILE_DIR:Lyrics-viewer>	

	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/resources"
	$<TARGET_FILE_DIR:Lyrics-viewer>/resources
)