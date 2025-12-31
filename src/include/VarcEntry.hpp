/**
 * @file VarcEntry.hpp
 * @brief VaultArchive file entry structure and utilities
 * @author LotusOS Core
 * @version 1.0.0
 */

#ifndef VARCENTRY_HPP
#define VARCENTRY_HPP

#include "VarcHeader.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

namespace VaultArchive {

    /**
     * @brief Per-entry flag definitions
     */
    struct EntryFlags {
        static constexpr uint32_t COMPRESSED = 0x0001;     // Entry is compressed
        static constexpr uint32_t ENCRYPTED = 0x0002;      // Entry is encrypted
        static constexpr uint32_t DIRECTORY = 0x0004;      // Entry is a directory
        static constexpr uint32_t SYMLINK = 0x0008;        // Entry is a symbolic link
        static constexpr uint32_t HIDDEN = 0x0010;         // Entry is hidden
        static constexpr uint32_t READONLY = 0x0020;       // Entry is read-only
        static constexpr uint32_t RESERVED = 0xFFC0;       // Reserved for future use
    };

    /**
     * @brief Represents a single file or directory entry in the archive
     */
    class VarcEntry {
    public:
        /**
         * @brief Entry type enumeration
         */
        enum class Type {
            FILE,
            DIRECTORY,
            SYMLINK
        };

    private:
        std::string m_relativePath;      // Relative path within archive
        Type m_type;                      // Entry type
        uint64_t m_originalSize;          // Original file size in bytes
        uint64_t m_compressedSize;        // Compressed size (if compressed)
        uint64_t m_offset;                // Offset in archive file
        uint32_t m_fileType;              // File type identifier
        uint32_t m_flags;                 // Entry flags
        std::chrono::system_clock::time_point m_creationTime;
        std::chrono::system_clock::time_point m_modificationTime;
        std::vector<uint8_t> m_checksum; // SHA-256 checksum of original data
        std::vector<uint8_t> m_data;     // File data (loaded on demand)

    public:
        /**
         * @brief Default constructor
         */
        VarcEntry();

        /**
         * @brief Constructor for file entry
         * @param path Relative path within archive
         * @param data File content
         * @param type Entry type (default: FILE)
         */
        VarcEntry(const std::string& path, const std::vector<uint8_t>& data, Type type = Type::FILE);

        /**
         * @brief Constructor with metadata
         * @param path Relative path within archive
         * @param type Entry type
         * @param originalSize Original file size
         * @param fileType File type identifier
         */
        VarcEntry(const std::string& path, Type type, uint64_t originalSize, uint32_t fileType);

        /**
         * @brief Destructor
         */
        ~VarcEntry();

        /**
         * @brief Get the relative path of the entry
         * @return Path string
         */
        const std::string& getPath() const;

        /**
         * @brief Set the relative path
         * @param path New path
         */
        void setPath(const std::string& path);

        /**
         * @brief Get the entry type
         * @return Entry type
         */
        Type getType() const;

        /**
         * @brief Set the entry type
         * @param type New type
         */
        void setType(Type type);

        /**
         * @brief Get original file size
         * @return Size in bytes
         */
        uint64_t getOriginalSize() const;

        /**
         * @brief Set original file size
         * @param size Size in bytes
         */
        void setOriginalSize(uint64_t size);

        /**
         * @brief Get compressed file size
         * @return Compressed size in bytes
         */
        uint64_t getCompressedSize() const;

        /**
         * @brief Set compressed file size
         * @param size Compressed size in bytes
         */
        void setCompressedSize(uint64_t size);

        /**
         * @brief Get archive offset for this entry
         * @return Offset in bytes from archive start
         */
        uint64_t getOffset() const;

        /**
         * @brief Set archive offset for this entry
         * @param offset Offset in bytes
         */
        void setOffset(uint64_t offset);

        /**
         * @brief Get file type identifier
         * @return File type
         */
        uint32_t getFileType() const;

        /**
         * @brief Set file type identifier
         * @param type File type
         */
        void setFileType(uint32_t type);

        /**
         * @brief Get entry flags
         * @return Flags bitmask
         */
        uint32_t getFlags() const;

        /**
         * @brief Set entry flags
         * @param flags Flags bitmask
         */
        void setFlags(uint32_t flags);

        /**
         * @brief Check if entry is compressed
         * @return true if compressed
         */
        bool isCompressed() const;

        /**
         * @brief Check if entry is encrypted
         * @return true if encrypted
         */
        bool isEncrypted() const;

        /**
         * @brief Check if entry is a directory
         * @return true if directory
         */
        bool isDirectory() const;

        /**
         * @brief Check if entry is a symbolic link
         * @return true if symlink
         */
        bool isSymlink() const;

        /**
         * @brief Get creation time
         * @return Creation timestamp
         */
        std::chrono::system_clock::time_point getCreationTime() const;

        /**
         * @brief Set creation time
         * @param time Creation timestamp
         */
        void setCreationTime(std::chrono::system_clock::time_point time);

        /**
         * @brief Get modification time
         * @return Modification timestamp
         */
        std::chrono::system_clock::time_point getModificationTime() const;

        /**
         * @brief Set modification time
         * @param time Modification timestamp
         */
        void setModificationTime(std::chrono::system_clock::time_point time);

        /**
         * @brief Get the stored checksum
         * @return SHA-256 hash vector
         */
        const std::vector<uint8_t>& getChecksum() const;

        /**
         * @brief Set the stored checksum
         * @param checksum SHA-256 hash
         */
        void setChecksum(const std::vector<uint8_t>& checksum);

        /**
         * @brief Get entry data
         * @return File content vector
         */
        const std::vector<uint8_t>& getData() const;

        /**
         * @brief Set entry data
         * @param data File content
         */
        void setData(const std::vector<uint8_t>& data);

        /**
         * @brief Move data into entry (more efficient for large files)
         * @param data File content to move
         */
        void setData(std::vector<uint8_t>&& data);

        /**
         * @brief Clear entry data from memory
         */
        void clearData();

        /**
         * @brief Get entry header for serialization
         * @param pathLength Path length value (output)
         * @return Entry header structure
         */
        EntryHeader getEntryHeader(uint32_t& pathLength) const;

        /**
         * @brief Get the serialized path data
         * @return Path as byte vector
         */
        std::vector<uint8_t> getPathData() const;

        /**
         * @brief Calculate total entry size in archive (header + path + data + checksum)
         * @return Total size in bytes
         */
        uint64_t getTotalSize() const;

        /**
         * @brief Calculate compression ratio
         * @return Ratio as percentage (0.0 to 100.0+)
         */
        double getCompressionRatio() const;

        /**
         * @brief Get human-readable file type string
         * @return Type description
         */
        std::string getTypeString() const;

        /**
         * @brief Get human-readable size string
         * @return Formatted size (e.g., "1.5 MB")
         */
        std::string getSizeString() const;

        /**
         * @brief Get human-readable compressed size string
         * @return Formatted size
         */
        std::string getCompressedSizeString() const;

        /**
         * @brief Format bytes to human-readable string
         * @param bytes Size in bytes
         * @return Formatted size string
         */
        static std::string formatSize(uint64_t bytes);
    };

    /**
     * @brief Archive entry collection
     */
    using VarcEntryList = std::vector<VarcEntry>;

} // namespace VaultArchive

#endif // VARCENTRY_HPP
