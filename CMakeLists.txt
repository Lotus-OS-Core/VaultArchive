cmake_minimum_required(VERSION 3.16)
project(VaultArchive VERSION 0.3.27 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Note: Qt5 GUI has been disabled due to compilation issues
# The CLI tool (varc_tool) provides full functionality

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# Enable testing
enable_testing()

# Define archive sources
set(LIB_SOURCES
    src/lib/Archive.cpp
    src/lib/CryptoEngine.cpp
    src/lib/CompressionEngine.cpp
    src/lib/Header.cpp
    src/lib/VarcEntry.cpp
)

set(LIB_HEADERS
    src/include/VarcHeader.hpp
    src/include/VarcEntry.hpp
    src/include/CryptoEngine.hpp
    src/include/CompressionEngine.hpp
    src/include/Archive.hpp
)

# Create static library
add_library(varc STATIC ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(varc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
target_link_libraries(varc PRIVATE OpenSSL::Crypto ZLIB::ZLIB)

# Create CLI executable
add_executable(varc_tool src/main.cpp)
target_link_libraries(varc_tool PRIVATE varc)

# Installation rules
include(GNUInstallDirs)

install(TARGETS varc varc_tool
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY src/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/varc
)

# Install man page
install(FILES docs/varc.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# Uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

# Testing
add_test(NAME BasicTest COMMAND varc_tool --help)
