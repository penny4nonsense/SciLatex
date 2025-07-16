# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "C:/Users/jason/Dropbox/PC/Documents/SciLatex;C:/Users/jason/Dropbox/PC/Documents/SciLatex/build/Desktop_Qt_6_8_0_MSVC2022_64bit-Release")
set(CPACK_CMAKE_GENERATOR "Ninja")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "C:/Qt/Tools/CMake_64/share/cmake-3.29/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "SciLatex built using CMake")
set(CPACK_GENERATOR "TGZ")
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INSTALL_CMAKE_PROJECTS "C:/Users/jason/Dropbox/PC/Documents/SciLatex/build/Desktop_Qt_6_8_0_MSVC2022_64bit-Release;SciLatex;ALL;/")
set(CPACK_INSTALL_PREFIX "C:/Program Files (x86)/SciLatex")
set(CPACK_MODULE_PATH "C:/Users/jason/Dropbox/PC/Documents/SciLatex/third_party/curl/CMake;C:/Qt/6.8.1/msvc2022_64/lib/cmake/Qt6;C:/Qt/6.8.1/msvc2022_64/lib/cmake/Qt6/3rdparty/extra-cmake-modules/find-modules;C:/Qt/6.8.1/msvc2022_64/lib/cmake/Qt6/3rdparty/kwin")
set(CPACK_NSIS_DISPLAY_NAME "SciLatex 0.1.0-DEV")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_NSIS_PACKAGE_NAME "SciLatex 0.1.0-DEV")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OUTPUT_CONFIG_FILE "C:/Users/jason/Dropbox/PC/Documents/SciLatex/build/Desktop_Qt_6_8_0_MSVC2022_64bit-Release/CPackConfig.cmake")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "C:/Qt/Tools/CMake_64/share/cmake-3.29/Templates/CPack.GenericDescription.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SciLatex built using CMake")
set(CPACK_PACKAGE_FILE_NAME "SciLatex-0.1.0-DEV-win64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SciLatex 0.1.0-DEV")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "SciLatex 0.1.0-DEV")
set(CPACK_PACKAGE_NAME "SciLatex")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "Humanity")
set(CPACK_PACKAGE_VERSION "0.1.0-DEV")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0-DEV")
set(CPACK_RESOURCE_FILE_LICENSE "C:/Qt/Tools/CMake_64/share/cmake-3.29/Templates/CPack.GenericLicense.txt")
set(CPACK_RESOURCE_FILE_README "C:/Qt/Tools/CMake_64/share/cmake-3.29/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "C:/Qt/Tools/CMake_64/share/cmake-3.29/Templates/CPack.GenericWelcome.txt")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_7Z "ON")
set(CPACK_SOURCE_GENERATOR "7Z;ZIP")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "C:/Users/jason/Dropbox/PC/Documents/SciLatex/build/Desktop_Qt_6_8_0_MSVC2022_64bit-Release/CPackSourceConfig.cmake")
set(CPACK_SOURCE_ZIP "ON")
set(CPACK_SYSTEM_NAME "win64")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "win64")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "C:/Users/jason/Dropbox/PC/Documents/SciLatex/build/Desktop_Qt_6_8_0_MSVC2022_64bit-Release/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
