/**
 * @file main.cpp
 * @brief VaultArchive command-line interface
 * @author LotusOS Core
 * @version 0.3.27
 */

#include "Archive.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace VaultArchive;

// Forward declarations
void printHelp();
void printVersion();
void printProgress(uint64_t current, uint64_t total, uint64_t currentBytes,
    uint64_t totalBytes, const std::string& currentFile);

std::string getPassword(bool confirm = false);
bool parseCompressionLevel(const std::string& value, int& level);

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    std::string command;
    std::string archivePath;
    std::vector<std::string> inputPaths;
    std::string outputDir;
    std::string password;
    std::string pattern;

    // Options
    bool compress = true;
    int compressionLevel = 6;
    bool encrypt = false;
    bool overwrite = false;
    bool showDetails = true;
    bool showChecksums = false;
    bool showTimestamps = true;
    bool humanReadable = true;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h" || arg == "-?") {
            printHelp();
            return 0;
        }

        if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        }

        if (arg == "--no-compress") {
            compress = false;
            continue;
        }

        if (arg == "--compress-level") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --compress-level requires a value\n";
                return 1;
            }
            if (!parseCompressionLevel(argv[++i], compressionLevel)) {
                std::cerr << "Error: Invalid compression level\n";
                return 1;
            }
            continue;
        }

        if (arg == "--encrypt" || arg == "-e") {
            encrypt = true;
            continue;
        }

        if (arg == "--password" || arg == "-p") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --password requires a value\n";
                return 1;
            }
            password = argv[++i];
            continue;
        }

        if (arg == "--overwrite" || arg == "-o") {
            overwrite = true;
            continue;
        }

        if (arg == "--quiet" || arg == "-q") {
            continue;  // Suppress progress output
        }

        if (arg == "--raw") {
            showChecksums = false;
            showTimestamps = false;
            humanReadable = false;
            continue;
        }

        if (command.empty()) {
            command = arg;
        } else if (archivePath.empty()) {
            archivePath = arg;
        } else {
            inputPaths.push_back(arg);
        }
    }

    // Execute command
    if (command.empty() || command == "help") {
        printHelp();
        return 0;
    }

    try {
        Archive archive;

        if (command == "create" || command == "c" || command == "pack") {
            if (archivePath.empty() || inputPaths.empty()) {
                std::cerr << "Error: Missing required arguments\n";
                std::cerr << "Usage: varc create <archive.varc> <files...>\n";
                return 1;
            }

            // Get password if encryption requested
            if (encrypt && password.empty()) {
                password = getPassword(true);
            }

            CreateOptions options;
            options.compress = compress;
            options.compressionLevel = compressionLevel;
            options.encrypt = encrypt;
            options.password = password;

            // Create archive
            if (!archive.create(archivePath)) {
                std::cerr << "Error: Failed to create archive: " << archive.getLastError() << "\n";
                return 1;
            }

            archive.setProgressCallback(printProgress);

            ArchiveResult result = archive.addFiles(inputPaths, options);

            if (!archive.save()) {
                std::cerr << "Error: Failed to save archive: " << archive.getLastError() << "\n";
                return 1;
            }

            std::cout << "\nCreated: " << archivePath << "\n";
            std::cout << "Files: " << result.filesProcessed << "\n";
            std::cout << "Size: " << std::fixed << std::setprecision(2)
                      << static_cast<double>(result.bytesProcessed) / 1024.0 << " KB\n";

            if (encrypt) {
                std::cout << "Encryption: AES-256-CBC\n";
            }

        } else if (command == "extract" || command == "x" || command == "unpack") {
            if (archivePath.empty()) {
                std::cerr << "Error: Missing archive path\n";
                std::cerr << "Usage: varc extract <archive.varc> [output_dir]\n";
                return 1;
            }

            // Third argument is output directory for extract command
            if (!inputPaths.empty()) {
                outputDir = inputPaths[0];
                inputPaths.clear();
            }

            if (outputDir.empty()) {
                outputDir = ".";
            }

            // Open archive
            if (!archive.open(archivePath, password)) {
                std::cerr << "Error: Failed to open archive: " << archive.getLastError() << "\n";
                return 1;
            }

            archive.setProgressCallback(printProgress);

            ExtractOptions options;
            options.outputDirectory = outputDir;
            options.overwrite = overwrite;

            ArchiveResult result = archive.extractAll(outputDir, password, options);

            if (!result.success) {
                std::cerr << "Warning: Some files may not have been extracted\n";
            }

            std::cout << "\nExtracted: " << result.filesProcessed << " files\n";
            std::cout << "Output: " << outputDir << "\n";

        } else if (command == "list" || command == "l") {
            if (archivePath.empty()) {
                std::cerr << "Error: Missing archive path\n";
                std::cerr << "Usage: varc list <archive.varc>\n";
                return 1;
            }

            if (!archive.open(archivePath)) {
                std::cerr << "Error: Failed to open archive: " << archive.getLastError() << "\n";
                return 1;
            }

            ListOptions options;
            options.showDetails = showDetails;
            options.showChecksums = showChecksums;
            options.showTimestamps = showTimestamps;
            options.humanReadable = humanReadable;

            std::cout << archive.list(options);

        } else if (command == "verify" || command == "v") {
            if (archivePath.empty()) {
                std::cerr << "Error: Missing archive path\n";
                std::cerr << "Usage: varc verify <archive.varc>\n";
                return 1;
            }

            if (!archive.open(archivePath, password)) {
                std::cerr << "Error: Failed to open archive: " << archive.getLastError() << "\n";
                return 1;
            }

            std::cout << archive.getVerificationReport(password) << "\n";

            if (archive.verify(password)) {
                std::cout << "Status: VERIFIED\n";
                return 0;
            } else {
                std::cout << "Status: FAILED\n";
                std::cerr << "Error: " << archive.getLastError() << "\n";
                return 2;
            }

        } else if (command == "add" || command == "a") {
            if (archivePath.empty() || inputPaths.empty()) {
                std::cerr << "Error: Missing required arguments\n";
                std::cerr << "Usage: varc add <archive.varc> <files...>\n";
                return 1;
            }

            if (!archive.open(archivePath, password)) {
                std::cerr << "Error: Failed to open archive: " << archive.getLastError() << "\n";
                return 1;
            }

            archive.setProgressCallback(printProgress);

            CreateOptions options;
            options.compress = compress;
            options.encrypt = !password.empty();
            options.password = password;

            ArchiveResult result = archive.addFiles(inputPaths, options);

            if (!archive.save()) {
                std::cerr << "Error: Failed to save archive: " << archive.getLastError() << "\n";
                return 1;
            }

            std::cout << "Added " << result.filesProcessed << " files to archive\n";

        } else if (command == "remove" || command == "rm") {
            if (archivePath.empty() || inputPaths.empty()) {
                std::cerr << "Error: Missing required arguments\n";
                std::cerr << "Usage: varc remove <archive.varc> <patterns...>\n";
                return 1;
            }

            if (!archive.open(archivePath, password)) {
                std::cerr << "Error: Failed to open archive: " << archive.getLastError() << "\n";
                return 1;
            }

            uint64_t removed = 0;
            for (const auto& pattern : inputPaths) {
                removed += archive.removeEntries(pattern);
            }

            if (!archive.save()) {
                std::cerr << "Error: Failed to save archive: " << archive.getLastError() << "\n";
                return 1;
            }

            std::cout << "Removed " << removed << " entries from archive\n";

        } else if (command == "lock") {
            if (archivePath.empty()) {
                std::cerr << "Error: Missing archive path\n";
                std::cerr << "Usage: varc lock <archive.varc>\n";
                return 1;
            }

            if (password.empty()) {
                password = getPassword(true);
            }

            if (!archive.open(archivePath)) {
                std::cerr << "Error: Failed to open archive: " << archive.getLastError() << "\n";
                return 1;
            }

            if (!archive.lock(password)) {
                std::cerr << "Error: Failed to lock archive: " << archive.getLastError() << "\n";
                return 1;
            }

            if (!archive.save()) {
                std::cerr << "Error: Failed to save archive: " << archive.getLastError() << "\n";
                return 1;
            }

            std::cout << "Archive locked successfully\n";

        } else if (command == "unlock") {
            if (archivePath.empty()) {
                std::cerr << "Error: Missing archive path\n";
                std::cerr << "Usage: varc unlock <archive.varc>\n";
                return 1;
            }

            if (password.empty()) {
                password = getPassword(false);
            }

            if (!archive.open(archivePath, password)) {
                std::cerr << "Error: Failed to unlock archive: " << archive.getLastError() << "\n";
                return 1;
            }

            if (!archive.unlock(password)) {
                std::cerr << "Error: Failed to unlock archive: " << archive.getLastError() << "\n";
                return 1;
            }

            if (!archive.save()) {
                std::cerr << "Error: Failed to save archive: " << archive.getLastError() << "\n";
                return 1;
            }

            std::cout << "Archive unlocked successfully\n";

        } else {
            std::cerr << "Error: Unknown command: " << command << "\n";
            std::cerr << "Use 'varc --help' for usage information\n";
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

// ======================
// Helper Functions
// ======================

void printHelp() {
    std::cout << R"(
VaultArchive (VARC) - Secure Archive Tool
==========================================

USAGE:
    varc <command> [options] <archive.varc> [files...]

COMMANDS:
    create, c, pack   Create a new archive
    extract, x, unpack Extract files from archive
    list, l           List archive contents
    verify, v         Verify archive integrity
    add, a            Add files to existing archive
    remove, rm        Remove files from archive
    lock              Encrypt/lock archive with password
    unlock            Decrypt/unlock archive
    help              Show this help message
    version           Show version information

OPTIONS:
    --help, -h        Show help
    --version, -v     Show version
    --password, -p    Specify password for encryption
    --encrypt, -e     Enable encryption for archive
    --no-compress     Disable compression
    --compress-level  Set compression level (0-9)
                      0 = No compression
                      1-3 = Fast compression
                      6 = Default
                      9 = Best compression
    --overwrite, -o   Overwrite existing files
    --quiet, -q       Suppress progress output
    --raw             Raw output (no formatting)

EXAMPLES:
    # Create an archive
    varc create backup.varc ./documents

    # Create encrypted archive
    varc create --encrypt backup.varc ./documents

    # Extract archive
    varc extract backup.varc ./output

    # List contents
    varc list backup.varc

    # Verify integrity
    varc verify backup.varc

    # Add files to archive
    varc add backup.varc ./new_files

    # Lock archive with password
    varc lock secure.varc

    # Unlock archive
    varc unlock secure.varc

)";

    std::cout << "\nFor more information, see USER_GUIDE.md\n";
}

void printVersion() {
    std::cout << R"(
VaultArchive Version 0.3.27
===========================

Features:
  - AES-256-CBC encryption
  - Zlib compression (DEFLATE algorithm)
  - SHA-256 integrity verification
  - Multi-file archives
  - Cross-platform (Windows, Linux, macOS)
  - Qt5 GUI interface available

Build:
  C++17, OpenSSL, zlib, Qt5

)";
}

void printProgress(uint64_t current, uint64_t total, uint64_t currentBytes,
    uint64_t totalBytes, const std::string& currentFile) {

    static const int barWidth = 40;
    double progress = total > 0 ? static_cast<double>(current) / total : 0.0;
    int pos = static_cast<int>(barWidth * progress);

    // Clear line
    std::cout << "\r[";

    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }

    std::cout << "] " << static_cast<int>(progress * 100.0) << "%";

    if (!currentFile.empty()) {
        std::string name = currentFile;
        if (name.length() > 30) {
            name = "..." + name.substr(name.length() - 27);
        }
        std::cout << " " << name;
    }

    std::cout << std::flush;
}

std::string getPassword(bool confirm) {
    std::string password;
    std::string confirmPassword;

    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    if (confirm) {
        std::cout << "Confirm password: ";
        std::getline(std::cin, confirmPassword);

        if (password != confirmPassword) {
            std::cerr << "Error: Passwords do not match\n";
            exit(1);
        }
    }

    return password;
}

bool parseCompressionLevel(const std::string& value, int& level) {
    try {
        level = std::stoi(value);
        return level >= 0 && level <= 9;
    } catch (...) {
        return false;
    }
}
