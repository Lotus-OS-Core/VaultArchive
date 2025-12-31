/**
 * @file VarcEntry.cpp
 * @brief VaultArchive file entry implementation
 * @author LotusOS Core
 * @version 1.0.0
 */

#include "VarcEntry.hpp"
#include "VarcHeader.hpp"
#include "CryptoEngine.hpp"
#include "CompressionEngine.hpp"
#include <iomanip>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <cstring>

namespace VaultArchive {

    // ======================
    // VarcEntry Implementation
    // ======================

    VarcEntry::VarcEntry()
        : m_type(Type::FILE), m_originalSize(0), m_compressedSize(0), m_offset(0),
          m_fileType(0), m_flags(0) {
    }

    VarcEntry::VarcEntry(const std::string& path, const std::vector<uint8_t>& data, Type type)
        : m_relativePath(path), m_type(type), m_originalSize(data.size()),
          m_compressedSize(data.size()), m_offset(0), m_fileType(0), m_flags(0),
          m_data(data) {

        auto now = std::chrono::system_clock::now();
        m_creationTime = now;
        m_modificationTime = now;

        // Detect file type from content
        if (!data.empty() && type == Type::FILE) {
            m_fileType = FileType::detect(data.data(), data.size());
        }

        // Calculate checksum
        m_checksum = CryptoEngine::sha256(data);
    }

    VarcEntry::VarcEntry(const std::string& path, Type type, uint64_t originalSize, uint32_t fileType)
        : m_relativePath(path), m_type(type), m_originalSize(originalSize),
          m_compressedSize(originalSize), m_offset(0), m_fileType(fileType),
          m_flags(0) {

        auto now = std::chrono::system_clock::now();
        m_creationTime = now;
        m_modificationTime = now;
    }

    VarcEntry::~VarcEntry() {
        clearData();
    }

    const std::string& VarcEntry::getPath() const {
        return m_relativePath;
    }

    void VarcEntry::setPath(const std::string& path) {
        m_relativePath = path;
    }

    VarcEntry::Type VarcEntry::getType() const {
        return m_type;
    }

    void VarcEntry::setType(Type type) {
        m_type = type;

        if (type == Type::DIRECTORY) {
            m_flags |= EntryFlags::DIRECTORY;
        } else if (type == Type::SYMLINK) {
            m_flags |= EntryFlags::SYMLINK;
        }
    }

    uint64_t VarcEntry::getOriginalSize() const {
        return m_originalSize;
    }

    void VarcEntry::setOriginalSize(uint64_t size) {
        m_originalSize = size;
    }

    uint64_t VarcEntry::getCompressedSize() const {
        return m_compressedSize;
    }

    void VarcEntry::setCompressedSize(uint64_t size) {
        m_compressedSize = size;
    }

    uint64_t VarcEntry::getOffset() const {
        return m_offset;
    }

    void VarcEntry::setOffset(uint64_t offset) {
        m_offset = offset;
    }

    uint32_t VarcEntry::getFileType() const {
        return m_fileType;
    }

    void VarcEntry::setFileType(uint32_t type) {
        m_fileType = type;
    }

    uint32_t VarcEntry::getFlags() const {
        return m_flags;
    }

    void VarcEntry::setFlags(uint32_t flags) {
        m_flags = flags;
    }

    bool VarcEntry::isCompressed() const {
        return (m_flags & EntryFlags::COMPRESSED) != 0;
    }

    bool VarcEntry::isEncrypted() const {
        return (m_flags & EntryFlags::ENCRYPTED) != 0;
    }

    bool VarcEntry::isDirectory() const {
        return m_type == Type::DIRECTORY || (m_flags & EntryFlags::DIRECTORY) != 0;
    }

    bool VarcEntry::isSymlink() const {
        return m_type == Type::SYMLINK || (m_flags & EntryFlags::SYMLINK) != 0;
    }

    std::chrono::system_clock::time_point VarcEntry::getCreationTime() const {
        return m_creationTime;
    }

    void VarcEntry::setCreationTime(std::chrono::system_clock::time_point time) {
        m_creationTime = time;
    }

    std::chrono::system_clock::time_point VarcEntry::getModificationTime() const {
        return m_modificationTime;
    }

    void VarcEntry::setModificationTime(std::chrono::system_clock::time_point time) {
        m_modificationTime = time;
    }

    const std::vector<uint8_t>& VarcEntry::getChecksum() const {
        return m_checksum;
    }

    void VarcEntry::setChecksum(const std::vector<uint8_t>& checksum) {
        m_checksum = checksum;
    }

    const std::vector<uint8_t>& VarcEntry::getData() const {
        return m_data;
    }

    void VarcEntry::setData(const std::vector<uint8_t>& data) {
        m_data = data;
        m_originalSize = data.size();
        m_compressedSize = data.size();
        m_checksum = CryptoEngine::sha256(data);

        // Update file type if not set
        if (m_fileType == 0 && !data.empty()) {
            m_fileType = FileType::detect(data.data(), data.size());
        }
    }

    void VarcEntry::setData(std::vector<uint8_t>&& data) {
        m_data = std::move(data);
        m_originalSize = m_data.size();
        m_compressedSize = m_data.size();
        m_checksum = CryptoEngine::sha256(m_data);

        // Update file type if not set
        if (m_fileType == 0 && !m_data.empty()) {
            m_fileType = FileType::detect(m_data.data(), m_data.size());
        }
    }

    void VarcEntry::clearData() {
        if (!m_data.empty()) {
            CryptoEngine::secureWipe(m_data);
        }
        m_data.clear();
    }

    EntryHeader VarcEntry::getEntryHeader(uint32_t& pathLength) const {
        EntryHeader header;
        pathLength = static_cast<uint32_t>(m_relativePath.length());
        header.pathLength = pathLength;
        header.originalSize = m_originalSize;
        header.compressedSize = m_compressedSize;
        header.fileType = m_fileType;
        header.flags = m_flags;
        return header;
    }

    std::vector<uint8_t> VarcEntry::getPathData() const {
        std::vector<uint8_t> data(m_relativePath.begin(), m_relativePath.end());
        return data;
    }

    uint64_t VarcEntry::getTotalSize() const {
        uint32_t pathLen = static_cast<uint32_t>(m_relativePath.length());
        return EntryHeader::fixedSize() + pathLen + m_compressedSize + CHECKSUM_SIZE;
    }

    double VarcEntry::getCompressionRatio() const {
        if (m_originalSize == 0) {
            return 0.0;
        }
        return (100.0 * m_compressedSize) / m_originalSize;
    }

    std::string VarcEntry::getTypeString() const {
        switch (m_fileType) {
            case FileType::TEXT:
                return "Text";
            case FileType::BINARY:
                return "Binary";
            case FileType::IMAGE:
                return "Image";
            case FileType::AUDIO:
                return "Audio";
            case FileType::VIDEO:
                return "Video";
            case FileType::DOCUMENT:
                return "Document";
            case FileType::ARCHIVE:
                return "Archive";
            default:
                return "Unknown";
        }
    }

    std::string VarcEntry::getSizeString() const {
        return formatSize(m_originalSize);
    }

    std::string VarcEntry::getCompressedSizeString() const {
        return formatSize(m_compressedSize);
    }

    std::string VarcEntry::formatSize(uint64_t bytes) {
        static const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            ++unitIndex;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
        return oss.str();
    }

} // namespace VaultArchive
