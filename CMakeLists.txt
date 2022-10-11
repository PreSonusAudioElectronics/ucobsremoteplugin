cmake_minimum_required(VERSION 3.16...3.21)

project(ucobscontrolplugin VERSION 1.1.1)
add_library(${CMAKE_PROJECT_NAME} MODULE)

# Replace `Your Name Here` with the name (yours or your organization's) you want to see as the
# author of the plugin (in the plugin's metadata itself and in the installers)
set(PLUGIN_AUTHOR "James Inkster")

set(MACOS_BUNDLEID "com.presonus.${CMAKE_PROJECT_NAME}")

set(LINUX_MAINTAINER_EMAIL "jinkster@fender.com")

# Add your custom source files here - header files are optional and only required for visibility
# e.g. in Xcode or Visual Studio
target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/common.cpp
			src/enumerators.cpp
			src/frontend.cpp
			src/networkconnection.cpp
			src/networkserver.cpp
			src/obsobjects.cpp
			src/protocoladapter.cpp
			src/statistics.cpp
			src/ucobscontrolplugin.cpp
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

# Import libobs as main plugin dependency
find_package(libobs REQUIRED)
include(cmake/ObsPluginHelpers.cmake)

# Uncomment these lines if you want to use the OBS Frontend API in your plugin
find_package(obs-frontend-api REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)

# Uncomment those lines if you want to use Qt in your plugin
find_qt(COMPONENTS Widgets Core Network)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets Qt::Network)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-macros.generated.h)

# /!\ TAKE NOTE: No need to edit things past this point /!\

# --- Platform-independent build settings ---

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macos.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macos.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Qunused-arguments)
  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})




