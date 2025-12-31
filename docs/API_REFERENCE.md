# VaultArchive API Reference

**VaultArchive (VARC)** C++ library API documentation for developers integrating archive functionality into their applications.

## Table of Contents

1. [Overview](#overview)
2. [Core Classes](#core-classes)
3. [Header Structures](#header-structures)
4. [Entry Structures](#entry-structures)
5. [Options Structures](#options-structures)
6. [Utility Classes](#utility-classes)
7. [Error Handling](#error-handling)
8. [Thread Safety](#thread-safety)
9. [Examples](#examples)
10. [Integration Guide](#integration-guide)

---

## Overview

The VaultArchive library provides a C++17 interface for creating, reading, and manipulating VARC archive files. The library is designed with a clean, object-oriented API that handles all aspects of archive management.

### Namespace

All library components are in the `VaultArchive` namespace:

```cpp
#include <Archive.hpp>
using namespace VaultArchive;
```

### Quick Start

```cpp
#include <Archive.hpp>
#include <iostream>

int main() {
    // Create a new archive
    Archive archive;
    archive.create("output.varc");
    archive.addFile("document.txt");
    archive.save();

    // Open and extract an archive
    Archive reader;
    reader.open("output.varc");
    reader.extractAll("./output");

    return 0;
}
```

---

## Core Classes

### Archive

The main class for archive operations. Handles creation, reading, modification, and extraction.

**Header**: `Archive.hpp`

```cpp
class Archive {
public:
    Archive();
    explicit Archive(const std::string& filepath);
    ~Archive();

    // Lifecycle
    bool create(const std::string& filepath);
    bool open(const std::string& filepath, const std::string& password = "");
    void close();
    bool save(const std::string& filepath = "");

    // State
    bool isOpen() const;
    bool isModified() const;
    const std::string& getLastError() const;

    // Add files
    bool addFile(const std::string& filepath, const CreateOptions& options = CreateOptions());
    ArchiveResult addFiles(const std::vector<std::string>& files, const CreateOptions& options = CreateOptions());
    ArchiveResult addDirectory(const std::string& dirPath, const CreateOptions& options = CreateOptions());
    bool addVirtualFile(const std::string& virtualPath, const std::vector<uint8_t>& data, const CreateOptions& options = CreateOptions());

    // Remove files
    bool removeEntry(const std::string& path);
    uint64_t removeEntries(const std::string& pattern);
    void clearEntries();

    // Extract files
    ArchiveResult extractAll(const std::string& outputDir, const std::string& password = "", const ExtractOptions& options = ExtractOptions());
    bool extractFile(const std::string& path, const std::string& outputPath, const std::string& password = "");
    ArchiveResult extractPattern(const std::string& pattern, const std::string& outputDir, const std::string& password = "");

    // Query
    uint64_t getEntryCount() const;
    uint64_t getTotalOriginalSize() const;
    uint64_t getTotalCompressedSize() const;
    const VarcEntryList& getEntries() const;
    const VarcEntry* findEntry(const std::string& path) const;
    VarcEntryList findEntries(const std::string& pattern) const;
    bool entryExists(const std::string& path) const;

    // Verify
    bool verify(const std::string& password = "");
    bool verifyEntry(const std::string& path, const std::string& password = "");
    std::string getVerificationReport(const std::string& password = "");

    // Utility
    std::string list(const ListOptions& options = ListOptions()) const;
    void setProgressCallback(ProgressCallback callback);

    // Encryption
    bool lock(const std::string& password);
    bool unlock(const std::string& password);
    bool changePassword(const std::string& oldPassword, const std::string& newPassword);

    // Metadata
    const ArchiveMetadata& getMetadata() const;
    void setMetadata(const ArchiveMetadata& metadata);

    // Statistics
    CompressionStats getStatistics() const;
};
```

#### Usage Example

```cpp
// Create and populate archive
Archive archive;
archive.create("backup.varc");

CreateOptions options;
options.compress = true;
options.compressionLevel = 6;
options.encrypt = true;
options.password = "secure123";

archive.addDirectory("./data", options);
archive.save();

// Later, open and extract
Archive reader;
reader.open("backup.varc", "secure123");
reader.extractAll("./restored");
```

---

## Header Structures

### GlobalHeader

Contains archive-level metadata and configuration.

```cpp
struct GlobalHeader {
    std::array<uint8_t, 4> signature;    // Magic bytes: "VARC"
    uint16_t version;                     // Format version
    uint16_t flags;                       // Archive flags
    uint32_t fileCount;                   // Number of files
    std::array<uint8_t, 32> salt;        // PBKDF2 salt
    std::array<uint8_t, 16> iv;          // AES initialization vector
    uint64_t reserved;                    // Reserved for future use

    bool isValid() const;
    bool isEncrypted() const;
    bool isCompressed() const;
};
```

**Constants:**

```cpp
// Archive flags
constexpr uint16_t ENCRYPTED = 0x0001;
constexpr uint16_t COMPRESSED = 0x0002;
constexpr uint16_t HAS_METADATA = 0x0004;

// Size constants
constexpr size_t SALT_SIZE = 32;
constexpr size_t IV_SIZE = 16;
constexpr size_t CHECKSUM_SIZE = 32;
```

### EntryHeader

Contains per-file metadata.

```cpp
struct EntryHeader {
    uint32_t pathLength;          // Length of path string
    uint64_t originalSize;        // Original uncompressed size
    uint64_t compressedSize;      // Compressed size
    uint32_t fileType;            // File type identifier
    uint32_t flags;               // Per-entry flags

    static size_t fixedSize();
};
```

### ArchiveMetadata

Optional archive metadata.

```cpp
struct ArchiveMetadata {
    uint64_t creationTime;
    uint64_t modificationTime;
    std::string creator;
    std::string description;
    std::vector<std::pair<std::string, std::string>> customTags;
};
```

---

## Entry Structures

### VarcEntry

Represents a single file or directory within an archive.

```cpp
class VarcEntry {
public:
    enum class Type { FILE, DIRECTORY, SYMLINK };

    VarcEntry();
    VarcEntry(const std::string& path, const std::vector<uint8_t>& data, Type type = Type::FILE);
    VarcEntry(const std::string& path, Type type, uint64_t originalSize, uint32_t fileType);

    // Path
    const std::string& getPath() const;
    void setPath(const std::string& path);

    // Type
    Type getType() const;
    void setType(Type type);
    bool isDirectory() const;
    bool isSymlink() const;

    // Size
    uint64_t getOriginalSize() const;
    void setOriginalSize(uint64_t size);
    uint64_t getCompressedSize() const;
    void setCompressedSize(uint64_t size);

    // Offset
    uint64_t getOffset() const;
    void setOffset(uint64_t offset);

    // File type
    uint32_t getFileType() const;
    void setFileType(uint32_t type);

    // Flags
    uint32_t getFlags() const;
    void setFlags(uint32_t flags);
    bool isCompressed() const;
    bool isEncrypted() const;

    // Timestamps
    std::chrono::system_clock::time_point getCreationTime() const;
    void setCreationTime(std::chrono::system_clock::time_point time);
    std::chrono::system_clock::time_point getModificationTime() const;
    void setModificationTime(std::chrono::system_clock::time_point time);

    // Checksum
    const std::vector<uint8_t>& getChecksum() const;
    void setChecksum(const std::vector<uint8_t>& checksum);

    // Data
    const std::vector<uint8_t>& getData() const;
    void setData(const std::vector<uint8_t>& data);
    void setData(std::vector<uint8_t>&& data);
    void clearData();

    // Utility
    uint64_t getTotalSize() const;
    double getCompressionRatio() const;
    std::string getTypeString() const;
    std::string getSizeString() const;
    std::string getCompressedSizeString() const;
};
```

**File Type Constants:**

```cpp
struct FileType {
    static constexpr uint32_t UNKNOWN = 0;
    static constexpr uint32_t TEXT = 1;
    static constexpr uint32_t BINARY = 2;
    static constexpr uint32_t IMAGE = 3;
    static constexpr uint32_t AUDIO = 4;
    static constexpr uint32_t VIDEO = 5;
    static constexpr uint32_t DOCUMENT = 6;
    static constexpr uint32_t ARCHIVE = 7;

    static uint32_t detect(const uint8_t* data, size_t size);
};
```

---

## Options Structures

### CreateOptions

Options for creating or adding files.

```cpp
struct CreateOptions {
    bool compress = true;
    int compressionLevel = 6;
    bool encrypt = false;
    std::string password;
    bool followSymlinks = true;
    bool includeHidden = true;
    std::vector<std::string> excludePatterns;
    ArchiveMetadata metadata;
};
```

**Compression Levels:**

```cpp
struct CompressionLevel {
    static constexpr int NO_COMPRESSION = 0;
    static constexpr int FASTEST = 1;
    static constexpr int DEFAULT = 6;
    static constexpr int BEST = 9;
};
```

### ExtractOptions

Options for extracting files.

```cpp
struct ExtractOptions {
    bool overwrite = false;
    bool preservePermissions = true;
    bool preserveTimestamps = true;
    std::string outputDirectory = ".";
    std::vector<std::string> filter;
};
```

### ListOptions

Options for listing archive contents.

```cpp
struct ListOptions {
    bool showDetails = true;
    bool showChecksums = false;
    bool showTimestamps = true;
    bool humanReadable = true;
};
```

---

## Utility Classes

### CryptoEngine

Provides cryptographic operations.

```cpp
class CryptoEngine {
public:
    static constexpr size_t AES_KEY_SIZE = 32;
    static constexpr size_t AES_BLOCK_SIZE = 16;
    static constexpr int PBKDF2_ITERATIONS = 100000;

    CryptoEngine();
    ~CryptoEngine();

    void initialize(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);
    void initializeFromPassword(const std::string& password, const std::vector<uint8_t>& salt);
    bool isInitialized() const;
    void clear();

    // Key generation
    static std::vector<uint8_t> generateSalt(size_t size = 32);
    static std::vector<uint8_t> generateIV();
    static std::vector<uint8_t> generateRandom(size_t size);
    static std::vector<uint8_t> deriveKey(
        const std::string& password,
        const std::vector<uint8_t>& salt,
        int iterations = PBKDF2_ITERATIONS,
        size_t keySize = 32
    );

    // Encryption/Decryption
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext);

    // Authenticated encryption
    struct EncryptionResult {
        std::vector<uint8_t> ciphertext;
        std::vector<uint8_t> tag;
    };

    EncryptionResult encryptAuthenticated(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> decryptAuthenticated(
        const std::vector<uint8_t>& ciphertext,
        const std::vector<uint8_t>& tag
    );

    // Hashing
    static std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> sha256File(const std::string& filepath);
    static std::vector<uint8_t> hmacSha256(
        const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& key
    );
    static bool verifyChecksum(
        const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& storedChecksum
    );

    // Utility
    static void secureWipe(std::vector<uint8_t>& buffer);
    static std::string bytesToHex(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> hexToBytes(const std::string& hex);
    static std::string getKdfInfo();
};
```

### CompressionEngine

Provides compression and decompression operations.

```cpp
class CompressionEngine {
public:
    explicit CompressionEngine(int level = 6);
    ~CompressionEngine();

    bool initialize();
    bool isInitialized() const;
    void setCompressionLevel(int level);
    int getCompressionLevel() const;

    // Compression
    CompressionResult compress(const std::vector<uint8_t>& data);
    CompressionResult compressFile(const std::string& filepath);

    // Decompression
    DecompressionResult decompress(
        const std::vector<uint8_t>& compressedData,
        uint64_t expectedSize = 0
    );
    bool decompressToFile(
        const std::vector<uint8_t>& compressedData,
        const std::string& outputPath,
        uint64_t expectedSize = 0
    );

    // Streaming (memory efficient)
    template<typename InputCallback, typename OutputCallback>
    CompressionResult compressStreaming(InputCallback input, OutputCallback output);

    template<typename InputCallback, typename OutputCallback>
    DecompressionResult decompressStreaming(
        InputCallback input,
        OutputCallback output,
        uint64_t expectedSize = 0
    );

    // Directory
    CompressionStats compressDirectory(
        const std::string& inputDir,
        const std::string& outputFile
    );

    // Utility
    static std::string getLevelName(int level);
    static bool isCompressed(const std::vector<uint8_t>& data);
    static double estimateCompressionRatio(const std::vector<uint8_t>& data);
    static int getOptimalLevel(uint32_t dataType);
    static std::string getAlgorithmInfo();
};
```

### CompressionResult

```cpp
struct CompressionResult {
    std::vector<uint8_t> compressedData;
    uint64_t originalSize = 0;
    uint64_t compressedSize = 0;
    double compressionRatio = 0.0;
    bool success = false;
    std::string errorMessage;
};
```

### DecompressionResult

```cpp
struct DecompressionResult {
    std::vector<uint8_t> decompressedData;
    uint64_t originalSize = 0;
    uint64_t decompressedSize = 0;
    bool success = false;
    std::string errorMessage;
};
```

### CompressionStats

```cpp
struct CompressionStats {
    uint64_t totalOriginalSize = 0;
    uint64_t totalCompressedSize = 0;
    uint64_t filesProcessed = 0;
    uint64_t directoriesProcessed = 0;
    double averageCompressionRatio = 0.0;
    uint64_t timeMs = 0;

    std::string getSummary() const;
    double getSavingsPercentage() const;
};
```

### ArchiveResult

```cpp
struct ArchiveResult {
    bool success = false;
    std::string message;
    uint64_t filesProcessed = 0;
    uint64_t bytesProcessed = 0;
    uint64_t timeMs = 0;
    CompressionStats stats;
};
```

### ProgressCallback

Callback type for progress reporting.

```cpp
using ProgressCallback = std::function<void(
    uint64_t current,           // Current file number
    uint64_t total,             // Total files
    uint64_t currentBytes,      // Current bytes processed
    uint64_t totalBytes,        // Total bytes to process
    const std::string& currentFile  // Current file name
)>;
```

---

## Error Handling

The library uses C++ exceptions for error handling. All public methods that can fail will throw exceptions or return false with an error message.

### Exception Types

```cpp
#include <stdexcept>

// Standard exceptions
throw std::runtime_error("Error message");
throw std::invalid_argument("Invalid argument");
throw std::out_of_range("Out of range");
```

### Error Handling Pattern

```cpp
try {
    Archive archive;
    archive.create("output.varc");
    archive.addFile("data.txt");
    archive.save();
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}

// Or use return values
Archive archive;
if (!archive.create("output.varc")) {
    std::cerr << "Error: " << archive.getLastError() << std::endl;
    return 1;
}
```

---

## Thread Safety

The VaultArchive library is **not thread-safe**. Each `Archive` instance should be accessed from a single thread.

### Guidelines

1. **One thread per archive**: Each Archive instance should be used by only one thread
2. **Synchronize access**: Use mutexes if sharing Archive objects between threads
3. **Copy carefully**: Creating copies of Archive objects copies internal data
4. **Callback safety**: Ensure progress callbacks are thread-safe if called from multiple threads

### Thread-Safe Usage

```cpp
#include <mutex>

class ThreadSafeArchive {
private:
    std::mutex mtx;
    Archive archive;

public:
    bool addFile(const std::string& path, const CreateOptions& options = {}) {
        std::lock_guard<std::mutex> lock(mtx);
        return archive.addFile(path, options);
    }

    bool save(const std::string& filepath = "") {
        std::lock_guard<std::mutex> lock(mtx);
        return archive.save(filepath);
    }
};
```

---

## Examples

### Example 1: Creating an Archive

```cpp
#include <Archive.hpp>
#include <iostream>

int main() {
    using namespace VaultArchive;

    Archive archive;

    // Create new archive
    if (!archive.create("backup.varc")) {
        std::cerr << "Failed to create archive: " << archive.getLastError() << std::endl;
        return 1;
    }

    // Configure options
    CreateOptions options;
    options.compress = true;
    options.compressionLevel = 6;
    options.encrypt = true;
    options.password = "secure_password";

    // Set progress callback
    archive.setProgressCallback([](uint64_t current, uint64_t total,
        uint64_t currentBytes, uint64_t totalBytes, const std::string& file) {
        std::cout << "\rProgress: " << current << "/" << total << " - " << file << std::flush;
    });

    // Add files
    archive.addFile("document1.txt", options);
    archive.addFile("image.png", options);
    archive.addDirectory("./data", options);

    // Save archive
    if (!archive.save()) {
        std::cerr << "\nFailed to save: " << archive.getLastError() << std::endl;
        return 1;
    }

    std::cout << "\nArchive created successfully!" << std::endl;

    // Print statistics
    auto stats = archive.getStatistics();
    std::cout << "Files: " << stats.filesProcessed << std::endl;
    std::cout << "Original: " << stats.totalOriginalSize << " bytes" << std::endl;
    std::cout << "Compressed: " << stats.totalCompressedSize << " bytes" << std::endl;

    return 0;
}
```

### Example 2: Extracting an Archive

```cpp
#include <Archive.hpp>
#include <iostream>

int main() {
    using namespace VaultArchive;

    Archive archive;

    // Open encrypted archive
    if (!archive.open("backup.varc", "secure_password")) {
        std::cerr << "Failed to open: " << archive.getLastError() << std::endl;
        return 1;
    }

    // List contents
    std::cout << archive.list() << std::endl;

    // Verify integrity
    if (!archive.verify("secure_password")) {
        std::cerr << "Verification failed: " << archive.getLastError() << std::endl;
        return 1;
    }
    std::cout << "Archive verified successfully!" << std::endl;

    // Extract to output directory
    ExtractOptions options;
    options.outputDirectory = "./restored";
    options.overwrite = true;

    auto result = archive.extractAll("./restored", "secure_password", options);

    if (!result.success) {
        std::cerr << "Extraction completed with warnings" << std::endl;
    }

    std::cout << "Extracted " << result.filesProcessed << " files" << std::endl;

    return 0;
}
```

### Example 3: Using CryptoEngine Directly

```cpp
#include <CryptoEngine.hpp>
#include <iostream>
#include <string>

int main() {
    using namespace VaultArchive;

    CryptoEngine crypto;

    // Generate key from password
    std::string password = "my_secret_password";
    auto salt = CryptoEngine::generateSalt();
    auto key = CryptoEngine::deriveKey(password, salt);

    // Initialize with key and IV
    auto iv = CryptoEngine::generateIV();
    crypto.initialize(key, iv);

    // Original data
    std::vector<uint8_t> plaintext = {'H', 'e', 'l', 'l', 'o', ' ',
                                       'W', 'o', 'r', 'l', 'd', '!'};

    // Encrypt
    auto ciphertext = crypto.encrypt(plaintext);
    std::cout << "Encrypted " << plaintext.size() << " bytes to "
              << ciphertext.size() << " bytes" << std::endl;

    // Decrypt
    auto decrypted = crypto.decrypt(ciphertext);

    // Verify
    if (decrypted == plaintext) {
        std::cout << "Round-trip successful!" << std::endl;
    }

    // Clean up
    crypto.secureWipe(key);
    crypto.clear();

    return 0;
}
```

### Example 4: Using CompressionEngine

```cpp
#include <CompressionEngine.hpp>
#include <iostream>
#include <fstream>

int main() {
    using namespace VaultArchive;

    CompressionEngine compressor(CompressionLevel::BEST);

    // Read file
    std::ifstream file("large_file.bin", std::ios::binary);
    std::vector<uint8_t> data(
        std::istreambuf_iterator<char>(file),
        {}
    );
    file.close();

    std::cout << "Original size: " << data.size() << " bytes" << std::endl;

    // Compress
    auto result = compressor.compress(data);

    if (!result.success) {
        std::cerr << "Compression failed: " << result.errorMessage << std::endl;
        return 1;
    }

    std::cout << "Compressed size: " << result.compressedSize << " bytes" << std::endl;
    std::cout << "Compression ratio: " << result.compressionRatio << "%" << std::endl;

    // Save compressed data
    std::ofstream out("compressed.bin", std::ios::binary);
    out.write(
        reinterpret_cast<const char*>(result.compressedData.data()),
        result.compressedData.size()
    );
    out.close();

    // Decompress
    CompressionEngine decompressor;
    auto decompressResult = decompressor.decompress(result.compressedData, data.size());

    if (!decompressResult.success) {
        std::cerr << "Decompression failed: " << decompressResult.errorMessage << std::endl;
        return 1;
    }

    if (decompressResult.decompressedData == data) {
        std::cout << "Round-trip successful!" << std::endl;
    }

    return 0;
}
```

### Example 5: Searching Within Archive

```cpp
#include <Archive.hpp>
#include <iostream>

int main() {
    using namespace VaultArchive;

    Archive archive;
    archive.open("backup.varc");

    // Find all text files
    auto textFiles = archive.findEntries("*.txt");

    std::cout << "Found " << textFiles.size() << " text files:" << std::endl;
    for (const auto& entry : textFiles) {
        std::cout << "  " << entry.getPath() << " ("
                  << entry.getSizeString() << ")" << std::endl;
    }

    // Find files in specific directory
    auto configFiles = archive.findEntries("config/*");
    for (const auto& entry : configFiles) {
        std::cout << "Config: " << entry.getPath() << std::endl;
    }

    // Get specific file data
    auto entry = archive.findEntry("important.txt");
    if (entry) {
        auto data = archive.getEntryData("important.txt");
        std::cout << "Important.txt size: " << data.size() << " bytes" << std::endl;
    }

    return 0;
}
```

---

## Integration Guide

### Building with CMake

```cmake
# Find VaultArchive package
find_package(VaultArchive 1.0 REQUIRED)

# Your executable/target
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE VaultArchive::varc)
```

### Building with Makefile

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS = -lvarc -lssl -lcrypto -lz

my_app: main.cpp
    $(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
```

### Installing as a Subdirectory

```cmake
add_subdirectory(VaultArchive)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE varc)
```

### Linking Against Built Library

```bash
# After building VaultArchive
g++ -std=c++17 my_app.cpp -I VaultArchive/include \
    -L VaultArchive/build -lvarc \
    -lssl -lcrypto -lz \
    -o my_app

# Set library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:VaultArchive/build
./my_app
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-12-30 | Initial release |

---

**VaultArchive API Reference**  
**Version**: 1.0.0  
**Last Updated**: 2025-12-30
