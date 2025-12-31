/**
 * @file CompressionEngine.cpp
 * @brief Compression operations implementation
 * @author LotusOS Core
 * @version 0.3.27
 */

#include "CompressionEngine.hpp"
#include "VarcHeader.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <array>
#include <cmath>

// Include zlib header
#include <zlib.h>

namespace VaultArchive {

    // ======================
    // CompressionEngine Implementation
    // ======================

    CompressionEngine::CompressionEngine(int level)
        : m_compressionLevel(level), m_windowBits(15 + 16), m_initialized(false) {
        // 15 + 16 = 31, which is window bits for raw DEFLATE with gzip wrapper
        // For raw deflate: window bits = 15 + (negative value)
    }

    CompressionEngine::~CompressionEngine() {
    }

    bool CompressionEngine::initialize() {
        m_initialized = true;
        return true;
    }

    bool CompressionEngine::isInitialized() const {
        return m_initialized;
    }

    void CompressionEngine::setCompressionLevel(int level) {
        m_compressionLevel = std::max(0, std::min(9, level));
    }

    int CompressionEngine::getCompressionLevel() const {
        return m_compressionLevel;
    }

    CompressionResult CompressionEngine::compress(const std::vector<uint8_t>& data) {
        CompressionResult result;
        result.success = false;
        result.originalSize = data.size();
        result.compressedSize = 0;
        result.compressionRatio = 0.0;

        if (data.empty()) {
            result.success = true;
            result.compressedData = data;
            return result;
        }

        // Initialize zlib stream
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;

        int ret = deflateInit2(
            &strm,
            m_compressionLevel,
            Z_DEFLATED,
            m_windowBits,
            8,
            Z_DEFAULT_STRATEGY
        );

        if (ret != Z_OK) {
            result.errorMessage = "Failed to initialize compression";
            return result;
        }

        // Allocate output buffer (worst case: slightly larger than input)
        uLongf bufferSize = deflateBound(&strm, data.size());
        result.compressedData.resize(bufferSize);

        strm.next_in = const_cast<unsigned char*>(data.data());
        strm.avail_in = static_cast<uInt>(data.size());

        strm.next_out = result.compressedData.data();
        strm.avail_out = bufferSize;

        // Compress
        ret = deflate(&strm, Z_FINISH);

        if (ret != Z_STREAM_END) {
            result.errorMessage = "Compression failed";
            deflateEnd(&strm);
            return result;
        }

        result.compressedSize = strm.total_out;
        result.compressedData.resize(result.compressedSize);
        result.success = true;

        if (result.originalSize > 0) {
            result.compressionRatio = (100.0 * result.compressedSize) / result.originalSize;
        }

        deflateEnd(&strm);
        return result;
    }

    CompressionResult CompressionEngine::compressFile(const std::string& filepath) {
        CompressionResult result;

        // Read file
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            result.errorMessage = "Cannot open file: " + filepath;
            return result;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(size);
        if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
            result.errorMessage = "Failed to read file: " + filepath;
            return result;
        }

        file.close();

        // Compress data
        result = compress(data);
        return result;
    }

    DecompressionResult CompressionEngine::decompress(
        const std::vector<uint8_t>& compressedData,
        uint64_t expectedSize
    ) {
        DecompressionResult result;
        result.success = false;
        result.originalSize = expectedSize;
        result.decompressedSize = 0;

        if (compressedData.empty()) {
            result.success = true;
            return result;
        }

        // Initialize zlib stream
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;

        int ret = inflateInit2(&strm, m_windowBits);

        if (ret != Z_OK) {
            result.errorMessage = "Failed to initialize decompression";
            return result;
        }

        // If expected size is known, allocate exact buffer
        // Otherwise, start with compressed size (decompressed is usually larger)
        uLongf bufferSize = expectedSize > 0 ?
            static_cast<uLongf>(expectedSize) :
            static_cast<uLongf>(compressedData.size() * 2);

        if (bufferSize < compressedData.size()) {
            bufferSize = static_cast<uLongf>(compressedData.size() * 2);
        }

        result.decompressedData.resize(bufferSize);

        strm.next_in = const_cast<unsigned char*>(compressedData.data());
        strm.avail_in = static_cast<uInt>(compressedData.size());

        // Decompress
        std::vector<uint8_t> output;
        int ret2;

        do {
            strm.next_out = result.decompressedData.data() + result.decompressedSize;
            strm.avail_out = bufferSize - result.decompressedSize;

            ret2 = inflate(&strm, Z_NO_FLUSH);

            if (ret2 == Z_STREAM_ERROR || ret2 == Z_DATA_ERROR) {
                result.errorMessage = "Decompression failed: " + std::string(strm.msg);
                inflateEnd(&strm);
                return result;
            }

            result.decompressedSize = strm.total_out;

            // Grow buffer if needed
            if (ret2 != Z_STREAM_END && strm.avail_out == 0) {
                bufferSize *= 2;
                result.decompressedData.resize(bufferSize);
            }

        } while (ret2 != Z_STREAM_END);

        result.decompressedData.resize(result.decompressedSize);
        result.success = true;

        inflateEnd(&strm);
        return result;
    }

    bool CompressionEngine::decompressToFile(
        const std::vector<uint8_t>& compressedData,
        const std::string& outputPath,
        uint64_t expectedSize
    ) {
        DecompressionResult result = decompress(compressedData, expectedSize);

        if (!result.success) {
            return false;
        }

        // Write to file
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        file.write(
            reinterpret_cast<const char*>(result.decompressedData.data()),
            result.decompressedData.size()
        );

        file.close();
        return true;
    }

    CompressionStats CompressionEngine::compressDirectory(
        const std::string& inputDir,
        const std::string& outputFile
    ) {
        CompressionStats stats;
        stats.totalOriginalSize = 0;
        stats.totalCompressedSize = 0;
        stats.filesProcessed = 0;
        stats.directoriesProcessed = 0;
        stats.timeMs = 0;
        stats.averageCompressionRatio = 0.0;

        auto startTime = std::chrono::high_resolution_clock::now();

        try {
            // Iterate through directory
            for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
                if (entry.is_regular_file()) {
                    CompressionResult compResult = compressFile(entry.path().string());

                    if (compResult.success) {
                        stats.filesProcessed++;
                        stats.totalOriginalSize += compResult.originalSize;
                        stats.totalCompressedSize += compResult.compressedSize;
                    }
                } else if (entry.is_directory()) {
                    stats.directoriesProcessed++;
                }
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            stats.timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime
            ).count();

            if (stats.totalOriginalSize > 0) {
                stats.averageCompressionRatio = (100.0 * stats.totalCompressedSize) / stats.totalOriginalSize;
            }

        } catch (const std::exception& e) {
            // Handle error
        }

        return stats;
    }

    std::string CompressionEngine::getLevelName(int level) {
        if (level == 0) return "None";
        if (level <= 1) return "Fastest";
        if (level <= 3) return "Fast";
        if (level <= 6) return "Default";
        if (level <= 9) return "Best";
        return "Custom";
    }

    bool CompressionEngine::isCompressed(const std::vector<uint8_t>& data) {
        if (data.size() < 2) {
            return false;
        }

        // Check for gzip magic bytes (0x1F, 0x8B)
        if (data[0] == 0x1F && data[1] == 0x8B) {
            return true;
        }

        // Check for zlib magic bytes (0x78)
        if (data[0] == 0x78) {
            return true;
        }

        // Basic DEFLATE check
        // Check if first two bits are 0 (uncompressed) or 01 (fixed Huffman)
        // This is a heuristic and may produce false positives
        uint8_t firstByte = data[0];
        uint8_t secondByte = data[1];

        // Check for deflate block headers
        // BFINAL bit (lowest bit of first byte)
        bool bfinal = firstByte & 0x01;

        // BTYPE (bits 1-2 of first byte)
        int btype = (firstByte >> 1) & 0x03;

        // Valid BTYPE values: 00 (no compression), 01 (fixed Huffman), 10 (dynamic Huffman)
        if (bfinal && btype <= 2) {
            return true;
        }

        return false;
    }

    double CompressionEngine::estimateCompressionRatio(const std::vector<uint8_t>& data) {
        if (data.empty()) {
            return 100.0;  // Empty data compresses to nothing
        }

        // Heuristic based on data characteristics
        size_t uniqueBytes = 0;
        std::array<bool, 256> seen{};
        seen.fill(false);

        for (uint8_t byte : data) {
            if (!seen[byte]) {
                seen[byte] = true;
                uniqueBytes++;
            }
        }

        // Calculate entropy (simplified)
        double entropy = 0.0;
        for (bool present : seen) {
            if (present) {
                double p = static_cast<double>(uniqueBytes) / 256.0;
                entropy -= p * std::log2(p);
            }
        }

        // Estimate compression ratio based on entropy
        // High entropy = low compressibility (ratio close to 100%)
        // Low entropy = high compressibility (ratio much less than 100%)
        double estimatedRatio = entropy * 100.0 / 8.0;  // 8 bits per byte

        return std::min(100.0, estimatedRatio);
    }

    int CompressionEngine::getOptimalLevel(uint32_t dataType) {
        switch (dataType) {
            case FileType::TEXT:
                return CompressionLevel::BEST;  // Text compresses well
            case FileType::DOCUMENT:
                return CompressionLevel::BEST;  // Documents compress well
            case FileType::IMAGE:
                return CompressionLevel::DEFAULT;  // Images may already be compressed
            case FileType::AUDIO:
            case FileType::VIDEO:
                return CompressionLevel::FASTEST;  // Audio/video usually already compressed
            case FileType::BINARY:
            default:
                return CompressionLevel::DEFAULT;  // Default level for binary data
        }
    }

    // ======================
    // CompressionResult Implementation
    // ======================

    std::string CompressionResult::getSummary() const {
        std::ostringstream oss;
        oss << "Original: " << formatSize(originalSize) << "\n";
        oss << "Compressed: " << formatSize(compressedSize) << "\n";
        oss << "Ratio: " << std::fixed << std::setprecision(2) << compressionRatio << "%\n";
        return oss.str();
    }

    std::string CompressionResult::formatSize(uint64_t bytes) {
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

    // ======================
    // DecompressionResult Implementation
    // ======================

    std::string DecompressionResult::getSummary() const {
        std::ostringstream oss;
        oss << "Expected: " << formatSize(originalSize) << "\n";
        oss << "Decompressed: " << formatSize(decompressedSize) << "\n";
        return oss.str();
    }

    std::string DecompressionResult::formatSize(uint64_t bytes) {
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

    double CompressionStats::getSavingsPercentage() const {
        if (totalOriginalSize == 0) {
            return 0.0;
        }
        return 100.0 - averageCompressionRatio;
    }

    std::string CompressionStats::formatSize(uint64_t bytes) {
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
