cmake_minimum_required(VERSION 3.5)
project(ucobscontrolplugin VERSION 1.0.0)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DASIO_STANDALONE)

if (WIN32 OR APPLE)
	include(external/FindLibObs.cmake)
endif()

find_package(LibObs REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)

set(ucobscontrolplugin_SOURCES
	src/common.cpp
	src/enumerators.cpp
	src/frontend.cpp
	src/networkconnection.cpp
	src/networkserver.cpp
	src/obsobjects.cpp
	src/protocoladapter.cpp
	src/statistics.cpp
	src/ucobscontrolplugin.cpp)

set(ucobscontrolplugin_HEADERS
	src/common.h
	src/enumerators.h
	src/frontend.h
	src/networkconnection.h
	src/networkserver.h
	src/obsobjects.h
	src/obsremoteprotocol.h
	src/protocoladapter.h
	src/statistics.h
	src/ucobscontrolplugin.h)

# --- Platform-independent build settings ---
add_library(ucobscontrolplugin MODULE
	${ucobscontrolplugin_SOURCES}
	${ucobscontrolplugin_HEADERS})

include_directories(
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5Widgets_INCLUDES}
	${Qt5Network_INCLUDES})

target_link_libraries(ucobscontrolplugin
	libobs
	Qt5::Core
	Qt5::Widgets
	Qt5::Network)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(WIN32)
	if(NOT DEFINED OBS_FRONTEND_LIB)
		set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
		message(FATAL_ERROR "Could not find OBS Frontend API's library !")
	endif()

    if(MSVC)
        # Enable Multicore Builds and disable FH4 (to not depend on VCRUNTIME140_1.DLL)
	    add_definitions(/MP /d2FH4-)
    endif()

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH_NAME "64bit")
		set(OBS_BUILDDIR_ARCH "build64")
	else()
		set(ARCH_NAME "32bit")
		set(OBS_BUILDDIR_ARCH "build32")
	endif()

	include_directories(
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/UI"
	)

	target_link_libraries(ucobscontrolplugin
		"${OBS_FRONTEND_LIB}")

	# --- Release package helper ---
	# The "release" folder has a structure similar OBS' one on Windows
	set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")

	add_custom_command(TARGET ucobscontrolplugin POST_BUILD
		# If config is Release, package release files
		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:Release>==1 ("${CMAKE_COMMAND}" -E copy
			"$<TARGET_FILE:ucobscontrolplugin>"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		# In Release mode, copy Qt image format plugins
		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E copy
			"${QTDIR}/plugins/imageformats/qjpeg.dll"
			"${RELEASE_DIR}/bin/${ARCH_NAME}/imageformats/qjpeg.dll")
		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E copy
			"${QTDIR}/plugins/imageformats/qjpeg.dll"
			"${RELEASE_DIR}/bin/${ARCH_NAME}/imageformats/qjpeg.dll")

		# If config is RelWithDebInfo, package release files
		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${RELEASE_DIR}/data/obs-plugins/ucobscontrolplugin"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 ("${CMAKE_COMMAND}" -E copy_directory
			"${PROJECT_SOURCE_DIR}/data"
			"${RELEASE_DIR}/data/obs-plugins/ucobscontrolplugin")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 ("${CMAKE_COMMAND}" -E copy
			"$<TARGET_FILE:ucobscontrolplugin>"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 ("${CMAKE_COMMAND}" -E copy
			"$<TARGET_PDB_FILE:ucobscontrolplugin>"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		# Copy to obs-studio dev environment for immediate testing
		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:ucobscontrolplugin>"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_PDB_FILE:ucobscontrolplugin>"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}")
	)
	# --- End of sub-section ---

endif()
# --- End of section ---

# --- Linux-specific build settings and tasks ---
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	set_target_properties(ucobscontrolplugin PROPERTIES PREFIX "")
	target_link_libraries(ucobscontrolplugin obs-frontend-api)

	file(GLOB locale_files data/locale/*.ini)

	if(${USE_UBUNTU_FIX})
		install(TARGETS ucobscontrolplugin
			LIBRARY DESTINATION "/usr/lib/ucobscontrolplugin")
	endif()
	install(TARGETS ucobscontrolplugin
		LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/ucobscontrolplugin")

endif()
# --- End of section ---

# -- OS X specific build settings and tasks --
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fvisibility=default")

	set(CMAKE_SKIP_RPATH TRUE)
	set_target_properties(ucobscontrolplugin PROPERTIES PREFIX "")
	target_link_libraries(ucobscontrolplugin "${OBS_FRONTEND_LIB}")
endif()
# -- End of section --
