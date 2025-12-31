# VaultArchive (varc)

<div align="center">

**A secure, compressed archive format with command-line**

[![Version 0.3.27](https://img.shields.io/badge/Version-0.3.27-blue.svg)](https://github.com/Lotus-OS-Core/VaultArchive)
[![License MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Qt5](https://img.shields.io/badge/Qt-5-yellow.svg)](https://www.qt.io/qt-for-tutorials)

</div>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
  - [Linux/macOS](#linuxmacos)
  - [Windows](#windows)
  - [From Source](#from-source)
- [Quick Start](#quick-start)
  - [Command Line Interface](#command-line-interface)
  - [Graphical User Interface](#graphical-user-interface)
- [Usage](#usage)
  - [CLI Commands](#cli-commands)
  - [GUI Operations](#gui-operations)
  - [Examples](#examples)
- [File Format](#file-format)
- [Security](#security)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)
- [Support](#support)

---

## Overview

VaultArchive (VARC) is a secure file archiving utility that combines compression, encryption, and integrity verification in a single, easy-to-use package. It provides command-line interfaces, making it suitable for both power users and beginners.

The project was originally inspired by the need for a custom file format with the structure **Header → Value → Checksum**, and has evolved into a comprehensive archiving solution.

## Features

### Core Features
- **Secure Encryption**: AES-256-CBC encryption with PBKDF2-HMAC-SHA256 key derivation
- **Compression**: Zlib/DEFLATE compression with 9 configurable levels
- **Integrity Verification**: SHA-256 checksums for every file
- **Multi-file Support**: Archive unlimited files and directories
- **Cross-platform**: Works on Windows, Linux, and macOS

### Interfaces
- **CLI Tool**: Fast, scriptable command-line interface (`varc`)
- **GUI Application**: Modern Qt5-based graphical interface (`varc-gui`)
- **C++ Library**: Embed archive functionality in your own applications

### Advanced Features
- Password protection for individual archives
- Selective compression levels
- File type detection and classification
- Archive locking/unlocking without re-encoding
- Progress tracking for long operations

---

## Installation

### Linux/macOS

#### Using Package Manager (Coming Soon)

```bash
# Ubuntu/Debian
sudo apt install vaultarchive

# Fedora/RHEL
sudo dnf install vaultarchive

# macOS (Homebrew)
brew install vaultarchive
```

#### Manual Installation

```bash
# Clone the repository
git clone https://github.com/Lotus-OS-Core/VaultArchive.git
cd VaultArchive

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install (requires root)
sudo make install

# Verify installation
varc --version
```

### Windows

#### Using Installer (Coming Soon)

Download the installer from the [releases page](https://github.com/Lotus-OS-Core/VaultArchive/releases) and run it.

#### Manual Installation

```powershell
# Clone the repository
git clone https://github.com/Lotus-OS-Core/VaultArchive.git
cd VaultArchive

# Create build directory
mkdir build
cd build

# Configure with Visual Studio
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Install
cmake --install . --config Release
```

### From Source

#### Prerequisites

- **C++17 compiler**: GCC 8+, Clang 7+, or MSVC 2019+
- **CMake 3.16+**
- **OpenSSL** development libraries
- **zlib** development libraries
- **Qt5** (for GUI only)

#### Ubuntu/Debian

```bash
sudo apt-get install cmake build-essential libssl-dev zlib1g-dev qtbase5-dev
```

#### Fedora/RHEL

```bash
sudo dnf install cmake gcc-c++ openssl-devel zlib-devel qt5-qtbase-devel
```

#### macOS

```bash
brew install cmake openssl zlib qt5
export PKG_CONFIG_PATH="/usr/local/opt/openssl/lib/pkgconfig:/usr/local/opt/zlib/lib/pkgconfig:$PKG_CONFIG_PATH"
```

---

## Quick Start

### Command Line Interface

#### Create an Archive

```bash
# Create archive from files
varc create backup.varc ./documents

# Create archive with compression
varc create --compress-level 9 archive.varc ./large_files

# Create encrypted archive
varc create --encrypt archive.varc ./sensitive_files
```

#### Extract an Archive

```bash
# Extract to current directory
varc extract archive.varc

# Extract to specific directory
varc extract archive.varc ./output

# Extract encrypted archive
varc extract --password mypassword archive.varc ./output
```

#### List Contents

```bash
# List archive contents
varc list archive.varc

# List with detailed information
varc list archive.varc

# List with checksums
varc list --checksums archive.varc
```

### Graphical User Interface

Launch the GUI application:

```bash
# From terminal
varc-gui

# Or find it in your application menu
# VaultArchive GUI
```

The GUI provides:
- Menu-driven archive creation and extraction
- Drag-and-drop file support
- Progress dialogs for operations
- Archive contents viewer
- About and help dialogs

---

## Usage

### CLI Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `create` | `c`, `pack` | Create a new archive |
| `extract` | `x`, `unpack` | Extract files from archive |
| `list` | `l` | List archive contents |
| `verify` | `v` | Verify archive integrity |
| `add` | `a` | Add files to existing archive |
| `remove` | `rm` | Remove files from archive |
| `lock` | - | Encrypt/lock archive |
| `unlock` | - | Decrypt/unlock archive |
| `help` | - | Show help |
| `version` | - | Show version |

### CLI Options

| Option | Description |
|--------|-------------|
| `--help, -h` | Show help |
| `--version, -v` | Show version |
| `--password, -p <pass>` | Specify password |
| `--encrypt, -e` | Enable encryption |
| `--no-compress` | Disable compression |
| `--compress-level <0-9>` | Set compression level |
| `--overwrite, -o` | Overwrite existing files |
| `--quiet, -q` | Suppress progress output |

### GUI Operations

#### Creating an Archive

1. Click **File → New Archive** or the "New" toolbar button
2. Select output file and destination
3. Add files or directories to archive
4. Configure compression and encryption options
5. Click **Create**

#### Extracting an Archive

1. Click **File → Open Archive** or the "Open" toolbar button
2. Select the archive file
3. Click **Actions → Extract** or the "Extract" toolbar button
4. Choose output directory
5. Click **Extract**

#### Adding Files

1. Open an archive
2. Click **Actions → Add Files** or the "Add" toolbar button
3. Select files to add
4. Click **Add**

#### Verifying Integrity

1. Open an archive
2. Click **Actions → Verify**
3. View verification results

---

## Examples

### Basic Archive Operations

```bash
# Create a simple archive
varc create documents.varc report.txt budget.xlsx presentation.pptx

# Create a compressed archive
varc create --compress-level 9 photos_backup.varc ./vacation_photos

# Create encrypted backup
varc create --encrypt --password MySecret123 secure_backup.varc ./important_files

# List archive contents
varc list backup.varc

# Extract archive
varc extract backup.varc ./restored_files

# Verify archive
varc verify secure_backup.varc --password MySecret123
```

### Advanced Operations

```bash
# Add files to existing archive
varc add archive.varc new_document.pdf

# Remove files from archive
varc remove archive.varc "*.tmp"

# Lock archive with password
varc lock archive.varc

# Unlock archive
varc unlock archive.varc

# Change archive password
varc unlock --password oldpass archive.varc
varc lock --password newpass archive.varc
```

### Scripting Examples

```bash
#!/bin/bash
# Automated backup script

DATE=$(date +%Y%m%d)
ARCHIVE_NAME="backup_${DATE}.varc"
SOURCE_DIR="/path/to/backup"
PASSWORD="your_password_here"

# Create encrypted backup
varc create --encrypt --password "$PASSWORD" "$ARCHIVE_NAME" "$SOURCE_DIR"

# Verify backup
if varc verify --password "$PASSWORD" "$ARCHIVE_NAME"; then
    echo "Backup successful: $ARCHIVE_NAME"
else
    echo "Backup failed!"
    exit 1
fi
```

---

## File Format

### Archive Structure

```
+---------------------+
| Global Header       |  (64 bytes)
| - Signature "VARC"  |
| - Version (0.3.27)  |
| - Flags             |
| - File Count        |
| - Salt/IV           |
+---------------------+
| Entry 1 Header      |  (variable)
| - Path Length       |
| - Original Size     |
| - Compressed Size   |
| - File Type         |
| - Flags             |
+---------------------+
| Path String         |  (variable)
+---------------------+
| Compressed Data     |  (variable)
+---------------------+
| SHA-256 Checksum    |  (32 bytes)
+---------------------+
| Entry 2 Header      |  ...
| ...                 |
+---------------------+
```

### File Extension

- **.varc** - Standard VaultArchive file

### Header Structure

| Field | Size | Description |
|-------|------|-------------|
| Signature | 4 bytes | Magic bytes "VARC" |
| Version | 2 bytes | Format version |
| Flags | 2 bytes | Archive flags |
| File Count | 4 bytes | Number of entries |
| Salt | 32 bytes | PBKDF2 salt |
| IV | 16 bytes | AES initialization vector |
| Reserved | 8 bytes | Reserved for future use |

---

## Security

### Encryption Details

| Feature | Implementation |
|---------|----------------|
| Algorithm | AES-256-CBC |
| Key Derivation | PBKDF2-HMAC-SHA256 |
| Iterations | 100,000 (OWASP recommended) |
| Salt Size | 256 bits |
| IV Size | 128 bits |

### Best Practices

1. **Use strong passwords**: Minimum 12 characters with mixed case, numbers, and symbols
2. **Enable encryption**: Use `--encrypt` for sensitive data
3. **Verify archives**: Use `varc verify` after creation and transfer
4. **Test extraction**: Always test extraction before deleting originals
5. **Backup passwords**: Store passwords securely; lost passwords = lost data

### Performance

| Operation | Encryption | Compression |
|-----------|------------|-------------|
| Create | Slower | Varies by level |
| Extract | Slower | Varies by level |
| Memory | ~256 MB | ~64 MB buffer |

---

## Documentation

### User Guide

For detailed usage instructions, see [USER_GUIDE.md](docs/USER_GUIDE.md):

- Installation guide
- Command reference
- Examples and tutorials
- Troubleshooting

### API Reference

For developer documentation, see [API_REFERENCE.md](docs/API_REFERENCE.md):

- C++ library API
- Class documentation
- Code examples
- Integration guide

### Man Page

On Linux systems, access the man page:

```bash
man varc
```

Or view the source: [docs/varc.1](docs/varc.1)

---

## Contributing

Contributions are welcome! Please read our [Contributing Guide](CONTRIBUTING.md) for details.

### Ways to Contribute

- Report bugs
- Suggest features
- Submit pull requests
- Improve documentation
- Translate interfaces

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test your changes
5. Submit a pull request

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2025 LotusOS Core

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files, to deal in the Software
without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Support

### Getting Help

- **Issues**: [GitHub Issues](https://github.com/Lotus-OS-Core/VaultArchiveissues)
- **Discussions**: [GitHub Discussions](https://github.com/Lotus-OS-Core/VaultArchive/discussions)
- **Wiki**: [GitHub Wiki](https://github.com/Lotus-OS-Core/VaultArchive/wiki)

### Reporting Bugs

When reporting bugs, please include:

1. Operating system and version
2. VaultArchive version (`varc --version`)
3. Steps to reproduce the issue
4. Expected behavior
5. Actual behavior
6. Any error messages

---

<div align="center">

**VaultArchive v0.3.27** - Secure Your Files

[Homepage](https://github.com/Lotus-OS-Core/VaultArchive) |
[Download](https://github.com/Lotus-OS-Core/VaultArchive/releases) |
[Documentation](docs/)

</div>
