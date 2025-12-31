# VaultArchive User Guide

**VaultArchive (VARC)** is a secure, compressed archive format for bundling multiple files into a single encrypted and integrity-verified container.

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [Command Reference](#command-reference)
5. [Examples](#examples)
6. [File Format](#file-format)
7. [Security](#security)
8. [Troubleshooting](#troubleshooting)

---

## Introduction

VaultArchive provides a reliable solution for:

- **File Compression**: Reduce storage size using DEFLATE (zlib) compression
- **Encryption**: Protect data with AES-256-CBC encryption
- **Integrity Verification**: Detect corruption with SHA-256 checksums
- **Archive Management**: Create, extract, and modify archives

### Key Features

- Cross-platform support (Windows, Linux, macOS)
- Strong encryption using industry-standard algorithms
- Memory-efficient streaming for large files
- Open file format specification

---

## Installation

### Prerequisites

Before installing VaultArchive, ensure you have:

- **C++17 compiler** (GCC 8+, Clang 7+, or MSVC 2019+)
- **CMake 3.16 or higher**
- **OpenSSL** development libraries
- **zlib** development libraries

### Linux/macOS Installation

```bash
# Clone or navigate to the VaultArchive directory
cd VaultArchive

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Install (requires root privileges on Linux)
sudo make install

# Verify installation
varc --version
```

### Windows Installation

```powershell
# Open Developer Command Prompt for VS
cd VaultArchive

# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 16 2019" -A x64

# Build
cmake --build . --config Release

# Install
cmake --install . --config Release

# Verify
varc --version
```

### Installation from Package Managers

#### macOS (Homebrew)

```bash
brew install vaultarchive
```

#### Linux (Snap)

```bash
sudo snap install vaultarchive
```

---

## Quick Start

### Creating Your First Archive

```bash
# Create an archive from a directory
varc create my_files.varc ./documents

# Create an encrypted archive
varc create --encrypt secure_backup.varc ./documents

# Create with maximum compression
varc create --compress-level 9 archive.varc ./large_files
```

### Extracting Files

```bash
# Extract to current directory
varc extract archive.varc

# Extract to specific directory
varc extract archive.varc ./output

# Extract encrypted archive
varc extract --password mypassword archive.varc ./output
```

### Listing Contents

```bash
# List archive contents
varc list archive.varc

# Detailed listing with checksums
varc list --checksums archive.varc
```

---

## Command Reference

### create - Create a New Archive

```bash
varc create [options] <archive.varc> <files...>
```

**Options:**

| Option | Description |
|--------|-------------|
| `--encrypt, -e` | Enable encryption |
| `--password, -p <pass>` | Set encryption password |
| `--no-compress` | Disable compression |
| `--compress-level <0-9>` | Set compression level |
| `--overwrite, -o` | Overwrite existing archive |

**Examples:**

```bash
# Create archive from multiple files
varc create archive.varc file1.txt file2.txt file3.txt

# Create encrypted archive
varc create --encrypt backup.varc ./folder

# Create without compression
varc create --no-compress archive.varc ./files
```

### extract - Extract Files from Archive

```bash
varc extract [options] <archive.varc> [output_dir]
```

**Options:**

| Option | Description |
|--------|-------------|
| `--password, -p <pass>` | Archive password |
| `--overwrite, -o` | Overwrite existing files |
| `--quiet, -q` | Suppress progress output |

**Examples:**

```bash
# Extract to current directory
varc extract archive.varc

# Extract to specific directory
varc extract archive.varc ./extracted

# Extract encrypted archive
varc extract --password secret archive.varc ./output
```

### list - List Archive Contents

```bash
varc list [options] <archive.varc>
```

**Options:**

| Option | Description |
|--------|-------------|
| `--checksums` | Show SHA-256 checksums |
| `--raw` | Raw output without formatting |

**Examples:**

```bash
# List with details
varc list archive.varc

# Show checksums
varc list --checksums archive.varc

# Compact listing
varc list --raw archive.varc
```

### verify - Verify Archive Integrity

```bash
varc verify [options] <archive.varc>
```

**Options:**

| Option | Description |
|--------|-------------|
| `--password, -p <pass>` | Archive password (if encrypted) |

**Examples:**

```bash
# Verify archive
varc verify archive.varc

# Verify encrypted archive
varc verify --password secret archive.varc
```

### add - Add Files to Archive

```bash
varc add [options] <archive.varc> <files...>
```

**Examples:**

```bash
# Add files to existing archive
varc add archive.varc new_file.txt

# Add multiple files
varc add archive.varc file1.txt file2.txt

# Add entire directory
varc add archive.varc ./new_folder
```

### remove - Remove Files from Archive

```bash
varc remove <archive.varc> <patterns...>
```

**Examples:**

```bash
# Remove specific file
varc remove archive.varc old_file.txt

# Remove using wildcard
varc remove archive.varc "*.tmp"

# Remove multiple patterns
varc remove archive.varc "*.log" "*.tmp" "cache/*"
```

### lock - Encrypt Existing Archive

```bash
varc lock <archive.varc>
```

**Examples:**

```bash
# Lock archive with password
varc lock archive.varc
# Enter password when prompted
```

### unlock - Decrypt Existing Archive

```bash
varc unlock <archive.varc>
```

**Examples:**

```bash
# Unlock archive
varc unlock archive.varc
# Enter password when prompted
```

---

## Examples

### Example 1: Backup with Encryption

```bash
# Create encrypted backup
varc create --encrypt --compress-level 9 \
    backup_$(date +%Y%m%d).varc \
    ~/Documents

# Verify the backup
varc verify backup_*.varc

# Extract when needed
varc extract --password your_password backup_*.varc ./restore
```

### Example 2: Incremental Backup Script

```bash
#!/bin/bash
# incremental_backup.sh

ARCHIVE="backup.varc"
SOURCE_DIR="./data"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

# Create temporary archive
varc create --compress-level 6 \
    "backup_$TIMESTAMP.varc" \
    "$SOURCE_DIR"

# Merge into main archive
varc add "$ARCHIVE" "backup_$TIMESTAMP.varc"

# Clean up temporary
rm "backup_$TIMESTAMP.varc"

echo "Backup complete: $ARCHIVE"
```

### Example 3: Secure File Transfer

```bash
# Sender side - create encrypted archive
varc create --encrypt transfer.varc ./files_to_send

# Recipient side - extract files
varc extract --password transfer_password transfer.varc ./received
```

### Example 4: Batch Processing

```bash
# Process multiple archives
for archive in *.varc; do
    echo "Processing: $archive"
    varc list "$archive" | head -20
done
```

---

## File Format

### Archive Structure

```
+---------------------+
| Global Header       |  (64 bytes)
| - Signature "VARC"  |
| - Version           |
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
| Entry 2 Header      |
| ...                 |
+---------------------+
```

### File Extensions

- **.varc** - Standard VaultArchive file
- **.vault** - Alternative extension (same format)

### Metadata

Each entry includes:

- File path (relative to archive root)
- Original file size
- Compressed file size
- File type detection
- Creation/modification timestamps
- SHA-256 checksum

---

## Security

### Encryption Details

VaultArchive uses **AES-256-CBC** for encryption with the following security features:

| Feature | Implementation |
|---------|----------------|
| Algorithm | AES-256-CBC |
| Key Derivation | PBKDF2-HMAC-SHA256 |
| Iterations | 100,000 (OWASP recommended) |
| Salt Size | 256 bits |
| IV Size | 128 bits |

### Password Requirements

- Minimum length: No minimum enforced (but recommended 8+ characters)
- Character set: Any Unicode characters
- Special characters: Supported

### Best Practices

1. **Use strong passwords**: Combine uppercase, lowercase, numbers, and symbols
2. **Enable encryption**: Use `--encrypt` for sensitive data
3. **Verify archives**: Use `varc verify` after creation and transfer
4. **Test extraction**: Always test extraction before deleting originals
5. **Backup passwords**: Store passwords securely; lost passwords = lost data

### Performance Considerations

| Operation | Encryption | Compression |
|-----------|------------|-------------|
| Create | Slower | Faster (level 1-3) |
| Extract | Slower | Slower (level 9) |
| Memory | ~256 MB | ~64 MB buffer |

---

## Troubleshooting

### Common Issues

#### "Cannot open archive file"

**Cause**: File doesn't exist or permissions denied

**Solution**:
```bash
# Check file exists
ls -la archive.varc

# Check permissions
chmod 644 archive.varc
```

#### "Password required for encrypted archive"

**Cause**: Archive is encrypted but no password provided

**Solution**:
```bash
varc extract --password your_password archive.varc
```

#### "Invalid archive signature"

**Cause**: File is not a valid VaultArchive or corrupted

**Solution**:
```bash
# Check file header
hexdump -C archive.varc | head

# Re-download if from network
# Recreate archive if local
```

#### "Decompression failed"

**Cause**: Corrupted archive or wrong password

**Solution**:
```bash
# Verify archive
varc verify archive.varc

# Try with password
varc verify --password archive.varc
```

#### "Insufficient memory"

**Cause**: Large archive with limited RAM

**Solution**:
- Use streaming extraction (built-in)
- Increase system swap space
- Process files in batches

### Getting Help

```bash
# Show help
varc --help

# Show version and features
varc --version

# Check documentation
cat docs/USER_GUIDE.md
cat docs/API_REFERENCE.md
```

### Reporting Issues

When reporting issues, include:

1. Command that failed
2. Complete error message
3. Operating system and version
4. VaultArchive version (`varc --version`)
5. Steps to reproduce

---

## Additional Resources

- **API Reference**: See [API_REFERENCE.md](API_REFERENCE.md)
- **Source Code**: https://github.com/Lotus-OS-Core/VaultArchive
- **Issues**: https://github.com/Lotus-OS-Core/VaultArchive/issues
- **Wiki**: https://github.com/Lotus-OS-Core/VaultArchive/wiki

---

**VaultArchive Version**: 1.0.0  
**Last Updated**: 2025-12-30
