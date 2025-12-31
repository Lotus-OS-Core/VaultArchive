#!/bin/bash
#
# VaultArchive Uninstallation Script
# Removes all installed VaultArchive files
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
BIN_DIR="${INSTALL_PREFIX}/bin"
LIB_DIR="${INSTALL_PREFIX}/lib"
INCLUDE_DIR="${INSTALL_PREFIX}/include"
MAN_DIR="${INSTALL_PREFIX}/share/man/man1"
PKGCONFIG_DIR="${LIB_DIR}/pkgconfig"

# Files to remove
BINARY_FILES=(
    "varc_tool"
    "varc"
)

LIBRARY_FILES=(
    "libvarc.a"
)

HEADER_FILES=(
    "VarcHeader.hpp"
    "VarcEntry.hpp"
    "CryptoEngine.hpp"
    "CompressionEngine.hpp"
    "Archive.hpp"
)

# Functions
print_header() {
    echo -e "\n${RED}========================================${NC}"
    echo -e "${RED}  VaultArchive Uninstaller${NC}"
    echo -e "${RED}========================================${NC}\n"
}

print_step() {
    echo -e "${GREEN}[-]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[x]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[i]${NC} $1"
}

confirm() {
    read -p "$1 [y/N]: " response
    case "$response" in
        [yY][eE][sS]|[yY])
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

remove_file() {
    local file="$1"
    local dir="$2"

    if [ -f "$dir/$file" ]; then
        rm -f "$dir/$file"
        print_step "Removed: $dir/$file"
        return 0
    else
        print_info "Not found: $dir/$file"
        return 1
    fi
}

remove_directory() {
    local dir="$1"

    if [ -d "$dir" ]; then
        # Check if directory is empty
        if [ -z "$(ls -A "$dir" 2>/dev/null)" ]; then
            rmdir "$dir" 2>/dev/null || true
            print_step "Removed empty directory: $dir"
        else
            print_warning "Directory not empty (may contain other files): $dir"
        fi
    fi
}

remove_headers() {
    print_step "Removing header files..."

    local removed=0
    local dir="$INCLUDE_DIR"

    # Try to remove individual headers
    for header in "${HEADER_FILES[@]}"; do
        if [ -f "$dir/$header" ]; then
            rm -f "$dir/$header"
            print_step "Removed: $dir/$header"
            ((removed++))
        elif [ -f "$dir/varc/$header" ]; then
            rm -f "$dir/varc/$header"
            print_step "Removed: $dir/varc/$header"
            ((removed++))
        fi
    done

    # Remove varc subdirectory if empty
    if [ -d "$dir/varc" ]; then
        if [ -z "$(ls -A "$dir/varc" 2>/dev/null)" ]; then
            rmdir "$dir/varc" 2>/dev/null || true
        fi
    fi

    print_info "Removed $removed header file(s)"
}

remove_pkg_config() {
    local pc_file="$PKGCONFIG_DIR/vaultarchive.pc"

    if [ -f "$pc_file" ]; then
        rm -f "$pc_file"
        print_step "Removed pkg-config file: $pc_file"

        # Remove empty pkgconfig directory
        if [ -d "$PKGCONFIG_DIR" ]; then
            if [ -z "$(ls -A "$PKGCONFIG_DIR" 2>/dev/null)" ]; then
                rmdir "$PKGCONFIG_DIR" 2>/dev/null || true
            fi
        fi
    fi
}

remove_man_page() {
    local man_file="$MAN_DIR/varc.1"

    if [ -f "$man_file" ]; then
        rm -f "$man_file"
        print_step "Removed man page: $man_file"
    fi
}

remove_cmake_config() {
    local cmake_dir="$LIB_DIR/cmake/vaultarchive"
    local cmake_file="$LIB_DIR/vaultarchiveConfig.cmake"

    if [ -d "$cmake_dir" ]; then
        rm -rf "$cmake_dir"
        print_step "Removed CMake config directory: $cmake_dir"
    fi

    if [ -f "$cmake_file" ]; then
        rm -f "$cmake_file"
        print_step "Removed CMake config file: $cmake_file"
    fi
}

update_ld_cache() {
    if [[ "$LIB_DIR" == /usr/lib* ]] || [[ "$LIB_DIR" == /lib* ]]; then
        if command -v ldconfig >/dev/null 2>&1; then
            print_info "Updating library cache..."
            ldconfig 2>/dev/null || true
        fi
    fi
}

count_removed() {
    local count=0

    for file in "${BINARY_FILES[@]}"; do
        [ -f "$BIN_DIR/$file" ] && ((count++))
    done

    for file in "${LIBRARY_FILES[@]}"; do
        [ -f "$LIB_DIR/$file" ] && ((count++))
    done

    echo $count
}

print_summary() {
    local removed=$1

    echo ""
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}  Uninstallation Complete!${NC}"
    echo -e "${RED}========================================${NC}"
    echo ""
    echo -e "  Files removed: ${BLUE}$removed${NC}"
    echo ""
    echo -e "${YELLOW}Note:${NC}"
    echo "  - Some configuration files may remain"
    echo "  - Your archives (.varc files) are not affected"
    echo "  - To reinstall, run: scripts/install.sh"
    echo ""
}

# Main uninstallation flow
main() {
    print_header

    # Parse arguments
    local skip_confirmation=false
    local purge=false

    while [[ $# -gt 0 ]]; do
        case $1 in
            --purge)
                purge=true
                shift
                ;;
            --skip-confirmation)
                skip_confirmation=true
                shift
                ;;
            --help|-h)
                echo "Usage: $0 [OPTIONS]"
                echo ""
                echo "Options:"
                echo "  --purge             Also remove configuration files"
                echo "  --skip-confirmation Skip confirmation prompt"
                echo "  --help, -h          Show this help"
                echo ""
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done

    # Count files to be removed
    files_to_remove=$(count_removed)

    if [ "$files_to_remove" -eq 0 ]; then
        print_warning "No VaultArchive files found to remove"
        print_info "The library may not be installed, or installed to a different location"
        echo ""
        echo "You can specify a custom installation prefix:"
        echo "  INSTALL_PREFIX=/opt/vaultarchive $0"
        exit 0
    fi

    print_info "Found $files_to_remove file(s) to remove"

    # Confirmation
    if [ "$skip_confirmation" = false ]; then
        echo ""
        if ! confirm "Remove VaultArchive installation?"; then
            echo "Uninstallation cancelled."
            exit 0
        fi
    fi

    # Uninstallation steps
    local total_removed=0

    echo ""
    print_step "Removing files..."

    # Remove binary files
    echo ""
    print_info "Binary files:"
    for file in "${BINARY_FILES[@]}"; do
        if remove_file "$file" "$BIN_DIR"; then
            ((total_removed++))
        fi
    done

    # Remove library files
    echo ""
    print_info "Library files:"
    for file in "${LIBRARY_FILES[@]}"; do
        if remove_file "$file" "$LIB_DIR"; then
            ((total_removed++))
        fi
    done

    # Remove headers
    echo ""
    remove_headers

    # Remove pkg-config
    echo ""
    remove_pkg_config

    # Remove man page
    echo ""
    remove_man_page

    # Remove CMake config
    echo ""
    remove_cmake_config

    # Update library cache
    echo ""
    update_ld_cache

    # Print summary
    print_summary $total_removed
}

# Run main
main "$@"
