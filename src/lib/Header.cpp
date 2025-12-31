/**
 * @file Header.cpp
 * @brief VaultArchive file format header implementation
 * @author LotusOS Core
 * @version 1.0.0
 */

#include "VarcHeader.hpp"
#include <cstring>
#include <ctime>
#include <algorithm>
#include <chrono>

namespace VaultArchive {

    // ======================
    // FileType Implementation
    // ======================

    uint32_t FileType::detect(const uint8_t* data, size_t size) {
        if (size < 4) {
            return UNKNOWN;
        }

        // Check for common magic bytes
        if (std::memcmp(data, "\x89PNG\r\n\x1a\n", 8) == 0) {
            return IMAGE;
        }

        if (std::memcmp(data, "GIF87a", 6) == 0 || std::memcmp(data, "GIF89a", 6) == 0) {
            return IMAGE;
        }

        if ((data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) ||
            (std::memcmp(data, "JFIF", 4) == 0) ||
            (std::memcmp(data, "Exif", 4) == 0)) {
            return IMAGE;
        }

        if (std::memcmp(data, "RIFF", 4) == 0 && size >= 12) {
            if (std::memcmp(data + 8, "WEBP", 4) == 0) {
                return IMAGE;
            }
        }

        // Audio/video magic bytes
        if (std::memcmp(data, "ID3", 3) == 0 || std::memcmp(data, "\xFF\xFB", 2) == 0 ||
            std::memcmp(data, "\xFF\xFA", 2) == 0 || std::memcmp(data, "OggS", 4) == 0) {
            return AUDIO;
        }

        if (std::memcmp(data, "\x00\x00\x00", 3) == 0 && size > 4) {
            if (data[4] == 'f' && data[5] == 't' && data[6] == 'y' && data[7] == 'p') {
                return VIDEO;
            }
        }

        // Document types
        if (std::memcmp(data, "%PDF", 4) == 0) {
            return DOCUMENT;
        }

        if (std::memcmp(data, "PK\x03\x04", 4) == 0 || std::memcmp(data, "PK\x05\x06", 4) == 0) {
            return ARCHIVE;
        }

        // Check if mostly printable ASCII (likely text)
        size_t printableCount = 0;
        size_t checkSize = std::min(size, static_cast<size_t>(256));

        for (size_t i = 0; i < checkSize; ++i) {
            if ((data[i] >= 32 && data[i] <= 126) || data[i] == '\n' || data[i] == '\r' || data[i] == '\t') {
                printableCount++;
            }
        }

        if (printableCount > checkSize * 0.9) {
            return TEXT;
        }

        return BINARY;
    }

    // ======================
    // GlobalHeader Implementation
    // ======================

    GlobalHeader::GlobalHeader() {
        signature = VARC_SIGNATURE;
        version = (VARC_VERSION_MAJOR << 8) | VARC_VERSION_MINOR;
        flags = 0;
        fileCount = 0;
        salt.fill(0);
        iv.fill(0);
        reserved = 0;
    }

    std::vector<uint8_t> GlobalHeader::serialize() const {
        std::vector<uint8_t> data;
        data.reserve(64);  // Fixed header size

        // Write signature
        data.insert(data.end(), signature.begin(), signature.end());

        // Write version
        data.push_back(static_cast<uint8_t>((version >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>(version & 0xFF));

        // Write flags
        data.push_back(static_cast<uint8_t>((flags >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>(flags & 0xFF));

        // Write file count
        for (int i = 3; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((fileCount >> (i * 8)) & 0xFF));
        }

        // Write salt (32 bytes)
        for (size_t i = 0; i < 32; ++i) {
            data.push_back(i < salt.size() ? salt[i] : 0);
        }

        // Write IV (16 bytes)
        for (size_t i = 0; i < 16; ++i) {
            data.push_back(i < iv.size() ? iv[i] : 0);
        }

        // Write reserved (8 bytes, big-endian)
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((reserved >> (i * 8)) & 0xFF));
        }

        return data;
    }

    bool GlobalHeader::deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < 64) {
            return false;
        }

        size_t offset = 0;

        // Read signature
        std::memcpy(signature.data(), data.data() + offset, 4);
        offset += 4;

        if (signature != VARC_SIGNATURE) {
            return false;
        }

        // Read version
        version = static_cast<uint16_t>(data[offset]) << 8;
        version |= static_cast<uint16_t>(data[offset + 1]);
        offset += 2;

        // Read flags
        flags = static_cast<uint16_t>(data[offset]) << 8;
        flags |= static_cast<uint16_t>(data[offset + 1]);
        offset += 2;

        // Read file count
        fileCount = 0;
        for (int i = 0; i < 4; ++i) {
            fileCount = (fileCount << 8) | data[offset + i];
        }
        offset += 4;

        // Read salt
        std::memcpy(salt.data(), data.data() + offset, SALT_SIZE);
        offset += SALT_SIZE;

        // Read IV
        std::memcpy(iv.data(), data.data() + offset, IV_SIZE);
        offset += IV_SIZE;

        // Read reserved
        reserved = 0;
        for (int i = 0; i < 8; ++i) {
            reserved = (reserved << 8) | data[offset + i];
        }

        return true;
    }

    bool GlobalHeader::isValid() const {
        return signature == VARC_SIGNATURE;
    }

    bool GlobalHeader::isEncrypted() const {
        return (flags & ArchiveFlags::ENCRYPTED) != 0;
    }

    bool GlobalHeader::isCompressed() const {
        return (flags & ArchiveFlags::COMPRESSED) != 0;
    }

    // ======================
    // EntryHeader Implementation
    // ======================

    EntryHeader::EntryHeader()
        : pathLength(0), originalSize(0), compressedSize(0), fileType(0), flags(0) {
    }

    std::vector<uint8_t> EntryHeader::serialize() const {
        std::vector<uint8_t> data;
        data.reserve(fixedSize());

        // Write path length (2 bytes)
        data.push_back(static_cast<uint8_t>((pathLength >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>(pathLength & 0xFF));

        // Write original size (8 bytes, big-endian)
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((originalSize >> (i * 8)) & 0xFF));
        }

        // Write compressed size (8 bytes, big-endian)
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((compressedSize >> (i * 8)) & 0xFF));
        }

        // Write file type (4 bytes)
        for (int i = 3; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((fileType >> (i * 8)) & 0xFF));
        }

        // Write flags (4 bytes)
        for (int i = 3; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((flags >> (i * 8)) & 0xFF));
        }

        return data;
    }

    bool EntryHeader::deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < fixedSize()) {
            return false;
        }

        size_t offset = 0;

        // Read path length (2 bytes)
        pathLength = static_cast<uint32_t>(data[offset]) << 8;
        pathLength |= static_cast<uint32_t>(data[offset + 1]);
        offset += 2;

        // Read original size (8 bytes)
        originalSize = 0;
        for (int i = 0; i < 8; ++i) {
            originalSize = (originalSize << 8) | data[offset + i];
        }
        offset += 8;

        // Read compressed size (8 bytes)
        compressedSize = 0;
        for (int i = 0; i < 8; ++i) {
            compressedSize = (compressedSize << 8) | data[offset + i];
        }
        offset += 8;

        // Read file type (4 bytes)
        fileType = 0;
        for (int i = 0; i < 4; ++i) {
            fileType = (fileType << 8) | data[offset + i];
        }
        offset += 4;

        // Read flags (4 bytes)
        flags = 0;
        for (int i = 0; i < 4; ++i) {
            flags = (flags << 8) | data[offset + i];
        }

        return true;
    }

    size_t EntryHeader::fixedSize() {
        return ENTRY_HEADER_SIZE;
    }

    // ======================
    // ArchiveMetadata Implementation
    // ======================

    ArchiveMetadata::ArchiveMetadata()
        : creationTime(0), modificationTime(0), creator(""), description("") {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();
        creationTime = timestamp;
        modificationTime = timestamp;
    }

    std::vector<uint8_t> ArchiveMetadata::serialize() const {
        std::vector<uint8_t> data;

        // Write creation time (8 bytes)
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((creationTime >> (i * 8)) & 0xFF));
        }

        // Write modification time (8 bytes)
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((modificationTime >> (i * 8)) & 0xFF));
        }

        // Write creator length and string
        uint32_t creatorLen = static_cast<uint32_t>(creator.length());
        for (int i = 3; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((creatorLen >> (i * 8)) & 0xFF));
        }
        data.insert(data.end(), creator.begin(), creator.end());

        // Write description length and string
        uint32_t descLen = static_cast<uint32_t>(description.length());
        for (int i = 3; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((descLen >> (i * 8)) & 0xFF));
        }
        data.insert(data.end(), description.begin(), description.end());

        // Write custom tags
        uint16_t tagCount = static_cast<uint16_t>(customTags.size());
        for (int i = 1; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((tagCount >> (i * 8)) & 0xFF));
        }

        for (const auto& tag : customTags) {
            // Write key length and key
            uint16_t keyLen = static_cast<uint16_t>(tag.first.length());
            for (int i = 1; i >= 0; --i) {
                data.push_back(static_cast<uint8_t>((keyLen >> (i * 8)) & 0xFF));
            }
            data.insert(data.end(), tag.first.begin(), tag.first.end());

            // Write value length and value
            uint16_t valueLen = static_cast<uint16_t>(tag.second.length());
            for (int i = 1; i >= 0; --i) {
                data.push_back(static_cast<uint8_t>((valueLen >> (i * 8)) & 0xFF));
            }
            data.insert(data.end(), tag.second.begin(), tag.second.end());
        }

        return data;
    }

    size_t ArchiveMetadata::deserialize(const std::vector<uint8_t>& data, size_t offset) {
        if (data.size() < offset + 16) {
            return 0;
        }

        // Read creation time (8 bytes)
        creationTime = 0;
        for (int i = 0; i < 8; ++i) {
            creationTime = (creationTime << 8) | data[offset + i];
        }
        offset += 8;

        // Read modification time (8 bytes)
        modificationTime = 0;
        for (int i = 0; i < 8; ++i) {
            modificationTime = (modificationTime << 8) | data[offset + i];
        }
        offset += 8;

        // Read creator
        uint32_t creatorLen = 0;
        for (int i = 0; i < 4; ++i) {
            creatorLen = (creatorLen << 8) | data[offset + i];
        }
        offset += 4;

        if (data.size() < offset + creatorLen) {
            return 0;
        }
        creator = std::string(reinterpret_cast<const char*>(data.data() + offset), creatorLen);
        offset += creatorLen;

        // Read description
        uint32_t descLen = 0;
        for (int i = 0; i < 4; ++i) {
            descLen = (descLen << 8) | data[offset + i];
        }
        offset += 4;

        if (data.size() < offset + descLen) {
            return 0;
        }
        description = std::string(reinterpret_cast<const char*>(data.data() + offset), descLen);
        offset += descLen;

        // Read custom tags
        uint16_t tagCount = 0;
        for (int i = 0; i < 2; ++i) {
            tagCount = (tagCount << 8) | data[offset + i];
        }
        offset += 2;

        for (uint16_t t = 0; t < tagCount; ++t) {
            if (data.size() < offset + 2) {
                return 0;
            }

            uint16_t keyLen = 0;
            for (int i = 0; i < 2; ++i) {
                keyLen = (keyLen << 8) | data[offset + i];
            }
            offset += 2;

            if (data.size() < offset + keyLen) {
                return 0;
            }
            std::string key(reinterpret_cast<const char*>(data.data() + offset), keyLen);
            offset += keyLen;

            if (data.size() < offset + 2) {
                return 0;
            }

            uint16_t valueLen = 0;
            for (int i = 0; i < 2; ++i) {
                valueLen = (valueLen << 8) | data[offset + i];
            }
            offset += 2;

            if (data.size() < offset + valueLen) {
                return 0;
            }
            std::string value(reinterpret_cast<const char*>(data.data() + offset), valueLen);
            offset += valueLen;

            customTags.emplace_back(key, value);
        }

        return offset;
    }

} // namespace VaultArchive
