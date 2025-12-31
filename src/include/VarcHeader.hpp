/**
 * @file VarcHeader.hpp
 * @brief VaultArchive file format header structures and parsing utilities
 * @author LotusOS Core
 * @version 1.0.0
 */

#ifndef VARCHEADER_HPP
#define VARCHEADER_HPP

#include <cstdint>
#include <array>
#include <string>
#include <vector>

namespace VaultArchive {

    /**
     * @brief Archive format version constants
     */
    constexpr uint16_t VARC_VERSION_MAJOR = 0;
    constexpr uint16_t VARC_VERSION_MINOR = 3;

    /**
     * @brief Archive format signature (magic bytes)
     */
    constexpr std::array<uint8_t, 4> VARC_SIGNATURE = {'V', 'A', 'R', 'C'};

    /**
     * @brief Size constants for fixed fields
     */
    constexpr size_t SALT_SIZE = 32;           // PBKDF2 salt size
    constexpr size_t IV_SIZE = 16;             // AES block size
    constexpr size_t CHECKSUM_SIZE = 32;       // SHA-256 hash size
    constexpr size_t ENTRY_HEADER_SIZE = 4 + 8 + 8 + 4 + 2; // Fixed part of entry header
    constexpr size_t MAX_PATH_LENGTH = 65535;  // Maximum file path length

    /**
     * @brief Archive flag definitions
     */
    struct ArchiveFlags {
        static constexpr uint16_t ENCRYPTED = 0x0001;  // Archive is encrypted
        static constexpr uint16_t COMPRESSED = 0x0002; // Archive uses compression
        static constexpr uint16_t HAS_METADATA = 0x0004; // Has custom metadata
        static constexpr uint16_t RESERVED = 0xFFF8;   // Reserved for future use
    };

    /**
     * @brief File type identifiers
     */
    struct FileType {
        static constexpr uint32_t UNKNOWN = 0;
        static constexpr uint32_t TEXT = 1;
        static constexpr uint32_t BINARY = 2;
        static constexpr uint32_t IMAGE = 3;
        static constexpr uint32_t AUDIO = 4;
        static constexpr uint32_t VIDEO = 5;
        static constexpr uint32_t DOCUMENT = 6;
        static constexpr uint32_t ARCHIVE = 7;

        /**
         * @brief Detect file type from content
         * @param data Pointer to file content
         * @param size Size of content
         * @return Detected file type
         */
        static uint32_t detect(const uint8_t* data, size_t size);
    };

    /**
     * @brief Global archive header structure
     * This structure is written at the beginning of every .varc file
     */
    struct GlobalHeader {
        std::array<uint8_t, 4> signature;    // Magic bytes: "VARC"
        uint16_t version;                     // Format version (major << 8 | minor)
        uint16_t flags;                       // Archive flags
        uint32_t fileCount;                   // Number of files in archive
        std::array<uint8_t, SALT_SIZE> salt; // Salt for key derivation (if encrypted)
        std::array<uint8_t, IV_SIZE> iv;      // Initialization vector (if encrypted)
        uint64_t reserved;                    // Reserved for future use

        /**
         * @brief Default constructor
         */
        GlobalHeader();

        /**
         * @brief Serialize header to byte vector
         * @return Serialized header data
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize header from byte vector
         * @param data Serialized header data
         * @return true if deserialization successful
         */
        bool deserialize(const std::vector<uint8_t>& data);

        /**
         * @brief Validate header signature and version
         * @return true if valid
         */
        bool isValid() const;

        /**
         * @brief Check if archive is encrypted
         * @return true if encrypted
         */
        bool isEncrypted() const;

        /**
         * @brief Check if archive uses compression
         * @return true if compressed
         */
        bool isCompressed() const;
    };

    /**
     * @brief File entry header structure
     * This structure precedes each file's data payload
     */
    struct EntryHeader {
        uint32_t pathLength;          // Length of relative path string
        uint64_t originalSize;        // Original uncompressed file size
        uint64_t compressedSize;      // Size after compression (or original if not compressed)
        uint32_t fileType;            // File type identifier
        uint32_t flags;               // Per-entry flags (reserved for future)

        /**
         * @brief Default constructor
         */
        EntryHeader();

        /**
         * @brief Serialize entry header to byte vector
         * @return Serialized header data
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize entry header from byte vector
         * @param data Serialized header data
         * @return true if deserialization successful
         */
        bool deserialize(const std::vector<uint8_t>& data);

        /**
         * @brief Get fixed header size
         * @return Size in bytes
         */
        static size_t fixedSize();
    };

    /**
     * @brief Archive metadata structure
     * Optional metadata stored after global header
     */
    struct ArchiveMetadata {
        uint64_t creationTime;        // Archive creation timestamp
        uint64_t modificationTime;    // Last modification timestamp
        std::string creator;          // Creator name/identifier
        std::string description;      // Archive description
        std::vector<std::pair<std::string, std::string>> customTags; // Key-value pairs

        /**
         * @brief Default constructor
         */
        ArchiveMetadata();

        /**
         * @brief Serialize metadata to byte vector
         * @return Serialized metadata
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize metadata from byte vector
         * @param data Serialized data
         * @param offset Starting offset in data
         * @return Number of bytes consumed
         */
        size_t deserialize(const std::vector<uint8_t>& data, size_t offset);
    };

} // namespace VaultArchive

#endif // VARCHEADER_HPP
