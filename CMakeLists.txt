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

# Minimal static libcurl build
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/curl)

add_subdirectory(sources)

# Install rules for the SciLatex application
install(TARGETS SciLatex
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
