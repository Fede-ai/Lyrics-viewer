cmake_minimum_required(VERSION 3.10)
project(Lyrics-viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Restricted build types" FORCE)

set(SOURCE
	src/authclient.cpp src/authclient.hpp 
	src/binding.hpp 
	src/button.hpp 
	src/cefapp.cpp src/cefapp.hpp 
	src/curlwrapper.cpp src/curlwrapper.hpp 
	src/json.hpp 
	src/main.cpp 
	src/overlay.cpp src/overlay.hpp
)
	
add_executable(Lyrics-viewer ${SOURCE})
set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT Lyrics-viewer)
target_sources(Lyrics-viewer PRIVATE ERRORS.md LICENSE.md README.md)
source_group("Documentation" FILES ERRORS.md LICENSE.md README.md)
if (MSVC)
	add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

set(PATH_SFML "C:/SFML-3.0.0" CACHE STRING "Path to the SFML library")
set(PATH_LIBCURL "C:/curl-8.12.1" CACHE STRING "Path to the libcurl library")
set(PATH_CEF "C:/cef_binary_131.2.7" CACHE STRING "Path to the cef library")

target_include_directories(Lyrics-viewer PRIVATE ${PATH_SFML}/include)
target_include_directories(Lyrics-viewer PRIVATE ${PATH_LIBCURL}/include)
target_include_directories(Lyrics-viewer PRIVATE ${PATH_CEF})

target_link_directories(Lyrics-viewer PRIVATE 
	${PATH_SFML}/lib
	${PATH_LIBCURL}/lib
	$<$<CONFIG:Release>:${PATH_CEF}/Release>
	$<$<CONFIG:Debug>:${PATH_CEF}/Debug>
)

target_link_libraries(Lyrics-viewer PRIVATE
	$<$<CONFIG:Release>:sfml-system.lib sfml-window.lib sfml-graphics.lib>
	$<$<CONFIG:Debug>:sfml-system-d.lib sfml-window-d.lib sfml-graphics-d.lib>
	libcurl.lib
	libcef.lib
	libcef_dll_wrapper.lib
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
	$<$<CONFIG:Release>:${PATH_CEF}/Release/chrome_elf.dll>
	$<$<CONFIG:Debug>:${PATH_CEF}/Debug/chrome_elf.dll>
	$<$<CONFIG:Release>:${PATH_CEF}/Release/libcef.dll>
	$<$<CONFIG:Debug>:${PATH_CEF}/Debug/libcef.dll>
	$<$<CONFIG:Release>:${PATH_CEF}/Release/libEGL.dll>
	$<$<CONFIG:Debug>:${PATH_CEF}/Debug/libEGL.dll>
	$<$<CONFIG:Release>:${PATH_CEF}/Release/libEGL.dll>
	$<$<CONFIG:Debug>:${PATH_CEF}/Debug/libEGL.dll>
	$<$<CONFIG:Release>:${PATH_CEF}/Release/libGLESv2.dll>
	$<$<CONFIG:Debug>:${PATH_CEF}/Debug/libGLESv2.dll>
	$<$<CONFIG:Release>:${PATH_CEF}/Release/v8_context_snapshot.bin>
	$<$<CONFIG:Debug>:${PATH_CEF}/Debug/v8_context_snapshot.bin>
	"${PATH_CEF}/Resources/chrome_100_percent.pak"
	"${PATH_CEF}/Resources/chrome_200_percent.pak"
	"${PATH_CEF}/Resources/icudtl.dat"
	"${PATH_CEF}/Resources/resources.pak"
	"${CMAKE_SOURCE_DIR}/dep/cacert.pem"
	$<TARGET_FILE_DIR:Lyrics-viewer>	
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/resources"
	$<TARGET_FILE_DIR:Lyrics-viewer>/resources
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PATH_CEF}/Resources/locales"
	$<TARGET_FILE_DIR:Lyrics-viewer>/locales
)

# add manifest
add_custom_command(TARGET Lyrics-viewer POST_BUILD
	COMMAND mt.exe -nologo -manifest 
	"${CMAKE_SOURCE_DIR}/dep/Lyrics-viewer.exe.manifest"
	"${CMAKE_SOURCE_DIR}/dep/compatibility.manifest" 
	$<$<CONFIG:Release>:-outputresource:"${CMAKE_BINARY_DIR}/Release/Lyrics-viewer.exe">
	$<$<CONFIG:Debug>:-outputresource:"${CMAKE_BINARY_DIR}/Debug/Lyrics-viewer.exe">
    COMMENT "Adding manifest..."
)