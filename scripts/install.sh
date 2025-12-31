#!/bin/bash
#
# VaultArchive Installation Script
# Installs VaultArchive binaries and libraries
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
BUILD_DIR="${PROJECT_DIR}/build"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
BIN_DIR="${INSTALL_PREFIX}/bin"
LIB_DIR="${INSTALL_PREFIX}/lib"
INCLUDE_DIR="${INSTALL_PREFIX}/include"
MAN_DIR="${INSTALL_PREFIX}/share/man/man1"

# Version info
VERSION="0.3.27"
PACKAGE_NAME="vaultarchive"

# Functions
print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}  VaultArchive ${VERSION} Installer${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

print_step() {
    echo -e "${GREEN}[+]${NC} $1"
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

check_dependencies() {
    print_step "Checking dependencies..."

    local missing_deps=()

    # Check for required tools
    command -v cmake >/dev/null 2>&1 || missing_deps+=("cmake")
    command -v make >/dev/null 2>&1 || missing_deps+=("make")
    command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || missing_deps+=("c++ compiler")

    # Check for libraries
    pkg-config --exists openssl 2>/dev/null || missing_deps+=("libssl-dev")
    pkg-config --exists zlib 2>/dev/null || missing_deps+=("zlib1g-dev")

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies:"
        for dep in "${missing_deps[@]}"; do
            echo "  - $dep"
        done
        echo ""
        print_info "To install dependencies on Ubuntu/Debian:"
        echo "    sudo apt-get install cmake build-essential libssl-dev zlib1g-dev"
        print_info "To install on Fedora/RHEL:"
        echo "    sudo dnf install cmake gcc-c++ openssl-devel zlib-devel"
        print_info "To install on Arch Linux:"
        echo "    sudo pacman -S cmake base-devel openssl zlib"
        exit 1
    fi

    print_step "All dependencies satisfied"
}

check_build() {
    print_step "Checking build directory..."

    if [ ! -d "$BUILD_DIR" ]; then
        print_warning "Build directory not found. Running CMake configuration..."
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"

        cmake .. \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
            || {
                print_error "CMake configuration failed"
                exit 1
            }
    fi

    cd "$BUILD_DIR"
}

build_project() {
    print_step "Building VaultArchive..."

    local cores=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    make -j$cores || {
        print_error "Build failed"
        exit 1
    }

    print_step "Build completed successfully"
}

check_installation() {
    print_step "Verifying installation..."

    local varc_binary="$BIN_DIR/varc_tool"
    local lib_file="$LIB_DIR/libvarc.a"

    if [ ! -f "$varc_binary" ]; then
        print_error "varc_tool not found at $varc_binary"
        return 1
    fi

    if [ ! -f "$lib_file" ]; then
        print_warning "Library file not found at $lib_file"
    fi

    print_step "Installation files verified"
    return 0
}

create_directories() {
    print_step "Creating installation directories..."

    mkdir -p "$BIN_DIR"
    mkdir -p "$LIB_DIR"
    mkdir -p "$INCLUDE_DIR"
    mkdir -p "$MAN_DIR"

    print_step "Directories created"
}

install_files() {
    print_step "Installing VaultArchive..."

    # Install binary
    print_info "Installing varc_tool to $BIN_DIR"
    install -m 755 "$BUILD_DIR/varc_tool" "$BIN_DIR/"

    # Create symlink
    ln -sf "$BIN_DIR/varc_tool" "$BIN_DIR/varc" 2>/dev/null || true

    # Install library
    if [ -f "$BUILD_DIR/libvarc.a" ]; then
        print_info "Installing libvarc.a to $LIB_DIR"
        install -m 644 "$BUILD_DIR/libvarc.a" "$LIB_DIR/"
    fi

    # Install headers
    if [ -d "$PROJECT_DIR/src/include" ]; then
        print_info "Installing headers to $INCLUDE_DIR"
        cp -r "$PROJECT_DIR/src/include/"* "$INCLUDE_DIR/" 2>/dev/null || \
            cp -r "$PROJECT_DIR/src/include/varc/"* "$INCLUDE_DIR/" 2>/dev/null || \
            cp -r "$PROJECT_DIR/src/include/." "$INCLUDE_DIR/"
    fi

    # Install man page if available
    if [ -f "$PROJECT_DIR/docs/varc.1" ]; then
        print_info "Installing man page"
        install -m 644 "$PROJECT_DIR/docs/varc.1" "$MAN_DIR/"
    fi

    # Install pkg-config file
    create_pkg_config

    print_step "Files installed successfully"
}

create_pkg_config() {
    print_info "Creating pkg-config file..."

    local pkgconfig_dir="$LIB_DIR/pkgconfig"
    mkdir -p "$pkgconfig_dir"

    cat > "$pkgconfig_dir/vaultarchive.pc" << EOF
prefix=$INSTALL_PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: VaultArchive
Description: Secure archive library with compression and encryption
Version: $VERSION
Libs: -L\${libdir} -lvarc
Cflags: -I\${includedir}
Requires: openssl zlib
EOF

    print_step "pkg-config file created"
}

update_ld_cache() {
    # Update library cache if installing to system directories
    if [[ "$LIB_DIR" == /usr/lib* ]] || [[ "$LIB_DIR" == /lib* ]]; then
        if command -v ldconfig >/dev/null 2>&1; then
            print_info "Updating library cache..."
            ldconfig 2>/dev/null || true
        fi
    fi
}

print_summary() {
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  Installation Complete!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    echo -e "  Version:      ${BLUE}$VERSION${NC}"
    echo -e "  Binary:       ${BLUE}$BIN_DIR/varc${NC}"
    echo -e "  Library:      ${BLUE}$LIB_DIR/libvarc.a${NC}"
    echo -e "  Headers:      ${BLUE}$INCLUDE_DIR${NC}"
    echo ""
    echo -e "${YELLOW}Quick Start:${NC}"
    echo ""
    echo -e "  Create archive:   ${GREEN}varc create archive.varc ./files${NC}"
    echo -e "  Extract archive:  ${GREEN}varc extract archive.varc ./output${NC}"
    echo -e "  List contents:    ${GREEN}varc list archive.varc${NC}"
    echo -e "  Get help:         ${GREEN}varc --help${NC}"
    echo ""
    echo -e "${YELLOW}Documentation:${NC}"
    echo ""
    echo -e "  User Guide:       ${BLUE}$PROJECT_DIR/docs/USER_GUIDE.md${NC}"
    echo -e "  API Reference:    ${BLUE}$PROJECT_DIR/docs/API_REFERENCE.md${NC}"
    echo ""
}

cleanup() {
    print_info "Cleaning up..."
}

# Main installation flow
main() {
    print_header

    # Parse arguments
    local skip_build=false
    local skip_confirmation=false

    while [[ $# -gt 0 ]]; do
        case $1 in
            --prefix=*)
                INSTALL_PREFIX="${1#*=}"
                shift
                ;;
            --skip-build)
                skip_build=true
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
                echo "  --prefix=<path>     Installation prefix (default: /usr/local)"
                echo "  --skip-build        Skip build step (assume already built)"
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

    # Print configuration
    print_info "Installation prefix: $INSTALL_PREFIX"
    print_info "Binary directory: $BIN_DIR"
    print_info "Library directory: $LIB_DIR"

    # Confirmation
    if [ "$skip_confirmation" = false ]; then
        echo ""
        if ! confirm "Proceed with installation?"; then
            echo "Installation cancelled."
            exit 0
        fi
    fi

    # Installation steps
    check_dependencies

    if [ "$skip_build" = false ]; then
        check_build
        build_project
    else
        print_warning "Skipping build step"
        cd "$BUILD_DIR"
    fi

    create_directories
    install_files
    update_ld_cache
    check_installation
    print_summary

    trap cleanup EXIT
}

# Run main
main "$@"
