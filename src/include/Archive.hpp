/**
 * @file Archive.hpp
 * @brief Main VaultArchive interface for creating and manipulating archives
 * @author LotusOS Core
 * @version 1.0.0
 */

#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include "VarcHeader.hpp"
#include "VarcEntry.hpp"
#include "CryptoEngine.hpp"
#include "CompressionEngine.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace VaultArchive {

    /**
     * @brief Archive operation progress callback
     * @param current Current file number
     * @param total Total number of files
     * @param currentBytes Current bytes processed
     * @param totalBytes Total bytes to process
     * @param currentFile Current file name
     */
    using ProgressCallback = std::function<void(
        uint64_t current,
        uint64_t total,
        uint64_t currentBytes,
        uint64_t totalBytes,
        const std::string& currentFile
    )>;

    /**
     * @brief Archive operation result
     */
    struct ArchiveResult {
        bool success;                          // Operation success
        std::string message;                   // Result message
        uint64_t filesProcessed;               // Files processed
        uint64_t bytesProcessed;               // Bytes processed
        uint64_t timeMs;                       // Time taken in milliseconds
        CompressionStats stats;                // Compression statistics

        /**
         * @brief Default constructor
         */
        ArchiveResult() : success(false), filesProcessed(0), bytesProcessed(0), timeMs(0) {}
    };

    /**
     * @brief Extract options
     */
    struct ExtractOptions {
        bool overwrite;                        // Overwrite existing files
        bool preservePermissions;              // Preserve file permissions
        bool preserveTimestamps;               // Preserve timestamps
        std::string outputDirectory;           // Output directory
        std::vector<std::string> filter;       // File name filters

        /**
         * @brief Default constructor
         */
        ExtractOptions() : overwrite(false), preservePermissions(true),
                           preserveTimestamps(true), outputDirectory(".") {}
    };

    /**
     * @brief Create options
     */
    struct CreateOptions {
        bool compress;                         // Enable compression
        int compressionLevel;                  // Compression level (0-9)
        bool encrypt;                          // Enable encryption
        std::string password;                  // Encryption password
        bool followSymlinks;                   // Follow symbolic links
        bool includeHidden;                    // Include hidden files
        std::vector<std::string> excludePatterns; // Patterns to exclude
        ArchiveMetadata metadata;              // Archive metadata

        /**
         * @brief Default constructor
         */
        CreateOptions() : compress(true), compressionLevel(6),
                          encrypt(false), followSymlinks(true),
                          includeHidden(true) {}
    };

    /**
     * @brief List options
     */
    struct ListOptions {
        bool showDetails;                      // Show detailed information
        bool showChecksums;                    // Show checksums
        bool showTimestamps;                   // Show timestamps
        bool humanReadable;                    // Human-readable sizes

        /**
         * @brief Default constructor
         */
        ListOptions() : showDetails(true), showChecksums(false),
                        showTimestamps(true), humanReadable(true) {}
    };

    /**
     * @brief Main archive class for VaultArchive operations
     *
     * This class provides the primary interface for creating, reading,
     * extracting, and manipulating VaultArchive archives.
     */
    class Archive {
    private:
        std::string m_filepath;                // Archive file path
        GlobalHeader m_header;                 // Archive header
        VarcEntryList m_entries;               // Archive entries
        std::vector<uint8_t> m_archiveData;    // In-memory archive data (for modifications)
        bool m_modified;                       // Modified flag
        bool m_loaded;                         // Loaded flag
        std::string m_errorMessage;            // Last error message

        // Crypto and compression engines
        std::unique_ptr<CryptoEngine> m_crypto;
        std::unique_ptr<CompressionEngine> m_compression;

        // Progress callback
        ProgressCallback m_progressCallback;

    public:
        /**
         * @brief Default constructor
         */
        Archive();

        /**
         * @brief Constructor with archive path
         * @param filepath Path to archive file
         */
        explicit Archive(const std::string& filepath);

        /**
         * @brief Destructor
         */
        ~Archive();

        // ======================
        // Lifecycle Methods
        // ======================

        /**
         * @brief Create a new empty archive
         * @param filepath Path for new archive
         * @return true if successful
         */
        bool create(const std::string& filepath);

        /**
         * @brief Open an existing archive
         * @param filepath Path to archive file
         * @param password Optional password for encrypted archives
         * @return true if successful
         */
        bool open(const std::string& filepath, const std::string& password = "");

        /**
         * @brief Close the archive and release resources
         */
        void close();

        /**
         * @brief Save modified archive
         * @param filepath Optional new filepath
         * @return true if successful
         */
        bool save(const std::string& filepath = "");

        /**
         * @brief Check if archive is open
         * @return true if archive is loaded
         */
        bool isOpen() const;

        /**
         * @brief Check if archive has been modified
         * @return true if modified
         */
        bool isModified() const;

        /**
         * @brief Get last error message
         * @return Error message string
         */
        const std::string& getLastError() const;

        // ======================
        // Create/Add Methods
        // ======================

        /**
         * @brief Add a single file to archive
         * @param filepath Path to file to add
         * @param options Create options
         * @return true if successful
         */
        bool addFile(const std::string& filepath, const CreateOptions& options = CreateOptions());

        /**
         * @brief Add multiple files to archive
         * @param files Vector of file paths
         * @param options Create options
         * @return Archive result
         */
        ArchiveResult addFiles(const std::vector<std::string>& files, const CreateOptions& options = CreateOptions());

        /**
         * @brief Add a directory recursively
         * @param dirPath Path to directory
         * @param options Create options
         * @return Archive result
         */
        ArchiveResult addDirectory(const std::string& dirPath, const CreateOptions& options = CreateOptions());

        /**
         * @brief Add data as a virtual file
         * @param virtualPath Virtual path within archive
         * @param data File data
         * @param options Create options
         * @return true if successful
         */
        bool addVirtualFile(
            const std::string& virtualPath,
            const std::vector<uint8_t>& data,
            const CreateOptions& options = CreateOptions()
        );

        /**
         * @brief Add entry directly
         * @param entry Entry to add
         * @param options Create options
         * @return true if successful
         */
        bool addEntry(const VarcEntry& entry, const CreateOptions& options = CreateOptions());

        // ======================
        // Remove Methods
        // ======================

        /**
         * @brief Remove an entry by path
         * @param path Path to entry
         * @return true if successful
         */
        bool removeEntry(const std::string& path);

        /**
         * @brief Remove entries matching pattern
         * @param pattern Glob-style pattern
         * @return Number of entries removed
         */
        uint64_t removeEntries(const std::string& pattern);

        /**
         * @brief Clear all entries
         */
        void clearEntries();

        // ======================
        // Extract Methods
        // ======================

        /**
         * @brief Extract all files
         * @param outputDir Output directory
         * @param password Optional password
         * @param options Extract options
         * @return Archive result
         */
        ArchiveResult extractAll(
            const std::string& outputDir,
            const std::string& password = "",
            const ExtractOptions& options = ExtractOptions()
        );

        /**
         * @brief Extract a single file
         * @param path Path to file in archive
         * @param outputPath Output file path
         * @param password Optional password
         * @return true if successful
         */
        bool extractFile(
            const std::string& path,
            const std::string& outputPath,
            const std::string& password = ""
        );

        /**
         * @brief Extract entries matching pattern
         * @param pattern Glob-style pattern
         * @param outputDir Output directory
         * @param password Optional password
         * @return Archive result
         */
        ArchiveResult extractPattern(
            const std::string& pattern,
            const std::string& outputDir,
            const std::string& password = ""
        );

        /**
         * @brief Get entry data
         * @param path Path to entry
         * @return Entry data vector (empty if not found)
         */
        std::vector<uint8_t> getEntryData(const std::string& path);

        // ======================
        // Query Methods
        // ======================

        /**
         * @brief Get number of entries
         * @return Number of entries
         */
        uint64_t getEntryCount() const;

        /**
         * @brief Get total original size
         * @return Total original size in bytes
         */
        uint64_t getTotalOriginalSize() const;

        /**
         * @brief Get total compressed size
         * @return Total compressed size in bytes
         */
        uint64_t getTotalCompressedSize() const;

        /**
         * @brief Get archive filepath
         * @return Archive filepath
         */
        const std::string& getFilepath() const;

        /**
         * @brief Get archive header
         * @return Const reference to header
         */
        const GlobalHeader& getHeader() const;

        /**
         * @brief Get all entries
         * @return Const reference to entries
         */
        const VarcEntryList& getEntries() const;

        /**
         * @brief Find entry by path
         * @param path Path to find
         * @return Pointer to entry or nullptr
         */
        const VarcEntry* findEntry(const std::string& path) const;

        /**
         * @brief Find entries matching pattern
         * @param pattern Glob-style pattern
         * @return Vector of matching entries
         */
        VarcEntryList findEntries(const std::string& pattern) const;

        /**
         * @brief Check if entry exists
         * @param path Path to check
         * @return true if exists
         */
        bool entryExists(const std::string& path) const;

        // ======================
        // Verification Methods
        // ======================

        /**
         * @brief Verify archive integrity
         * @param password Optional password
         * @return true if archive is valid
         */
        bool verify(const std::string& password = "");

        /**
         * @brief Verify specific entry
         * @param path Path to entry
         * @param password Optional password
         * @return true if entry is valid
         */
        bool verifyEntry(const std::string& path, const std::string& password = "");

        /**
         * @brief Get verification report
         * @param password Optional password
         * @return Report string
         */
        std::string getVerificationReport(const std::string& password = "");

        // ======================
        // Utility Methods
        // ======================

        /**
         * @brief List archive contents
         * @param options List options
         * @return Formatted listing string
         */
        std::string list(const ListOptions& options = ListOptions()) const;

        /**
         * @brief Set progress callback
         * @param callback Progress callback function
         */
        void setProgressCallback(ProgressCallback callback);

        /**
         * @brief Lock archive with password
         * @param password New password
         * @return true if successful
         */
        bool lock(const std::string& password);

        /**
         * @brief Unlock archive with password
         * @param password Current password
         * @return true if successful
         */
        bool unlock(const std::string& password);

        /**
         * @brief Change archive password
         * @param oldPassword Current password
         * @param newPassword New password
         * @return true if successful
         */
        bool changePassword(const std::string& oldPassword, const std::string& newPassword);

        /**
         * @brief Get archive metadata
         * @return Const reference to metadata
         */
        const ArchiveMetadata& getMetadata() const;

        /**
         * @brief Set archive metadata
         * @param metadata Metadata to set
         */
        void setMetadata(const ArchiveMetadata& metadata);

        /**
         * @brief Get compression statistics
         * @return Compression statistics
         */
        CompressionStats getStatistics() const;

        // ======================
        // Helper Methods
        // ======================

        /**
         * @brief Get human-readable total original size
         * @return Formatted size string
         */
        std::string getTotalOriginalSizeString() const;

        /**
         * @brief Get human-readable total compressed size
         * @return Formatted size string
         */
        std::string getTotalCompressedSizeString() const;

        /**
         * @brief Format size in human-readable form
         * @param bytes Size in bytes
         * @return Formatted size string
         */
        std::string formatSize(uint64_t bytes) const;

    private:
        // Internal methods
        bool readArchive(const std::string& password);
        bool writeArchive();
        bool processEntry(VarcEntry& entry, const CreateOptions& options);
        VarcEntry createEntryFromPath(const std::string& filepath);
        void updateHeader();
        void invokeProgress(uint64_t current, uint64_t total, uint64_t currentBytes, uint64_t totalBytes, const std::string& currentFile);
    };

} // namespace VaultArchive

#endif // ARCHIVE_HPP
