cmake_minimum_required(VERSION 3.16)

# Top-level project declaration
project(SciLatex VERSION 0.1 LANGUAGES CXX)

# Enable Qt's automatic handling of UIC, MOC, and RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify vcpkg toolchain file (if using vcpkg)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/jason/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Find necessary Qt modules
find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel Pdf PdfWidgets Core)

# Define project sources
set(SCILATEX_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    keyutils.h
    navigation.h navigation.cpp
    matrixmode.h matrixmode.cpp
    bracketchooserdialog.h bracketchooserdialog.cpp
    operatordialog.h operatordialog.cpp
    preferencesmanager.h preferencesmanager.cpp
    preferencesdialog.h preferencesdialog.cpp
    matlabutils.h matlabutils.cpp
    fontdialog.h fontdialog.cpp
)

# Minimal static libcurl build
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/curl)

# Add the SciLatex executable
qt_add_executable(SciLatex
    MANUAL_FINALIZATION
    ${SCILATEX_SOURCES}
    mathnamedialog.h mathnamedialog.cpp
    labeldialog.h labeldialog.cpp
    decoratordialog.h decoratordialog.cpp
    spacingdialog.h spacingdialog.cpp
    previewwindow.h previewwindow.cpp
    fontdialog.h fontdialog.cpp
    recentfilesdialog.h recentfilesdialog.cpp
)

target_include_directories(SciLatex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CURL_INCLUDE_DIR}
)

# Link SciLatex against required libraries
target_link_libraries(SciLatex PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
    Qt6::Pdf
    Qt6::PdfWidgets
    Qt6::Core
    libcurl
)

# Set application properties for SciLatex
set_target_properties(SciLatex PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Finalize the SciLatex executable for Qt 6
qt_finalize_executable(SciLatex)

# Install rules for the SciLatex application
install(TARGETS SciLatex
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
