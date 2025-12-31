/**
 * @file CompressionEngine.hpp
 * @brief Compression operations for VaultArchive
 * @author LotusOS Core
 * @version 0.3.27
 */

#ifndef COMPRESSIONENGINE_HPP
#define COMPRESSIONENGINE_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <zlib.h>

namespace VaultArchive {

    /**
     * @brief Compression level constants
     */
    struct CompressionLevel {
        static constexpr int NO_COMPRESSION = 0;
        static constexpr int FASTEST = 1;
        static constexpr int DEFAULT = 6;
        static constexpr int BEST = 9;
    };

    /**
     * @brief Result structure for compression operations
     */
    struct CompressionResult {
        std::vector<uint8_t> compressedData;   // Compressed data
        uint64_t originalSize;                  // Original uncompressed size
        uint64_t compressedSize;                // Compressed size
        double compressionRatio;                // Compression ratio (0.0 to 100.0+)
        bool success;                           // Operation success status
        std::string errorMessage;               // Error message if failed

        /**
         * @brief Get human-readable summary
         * @return Formatted summary string
         */
        std::string getSummary() const;

        /**
         * @brief Format size to human-readable string
         * @param bytes Size in bytes
         * @return Formatted size string
         */
        static std::string formatSize(uint64_t bytes);
    };

    /**
     * @brief Result structure for decompression operations
     */
    struct DecompressionResult {
        std::vector<uint8_t> decompressedData; // Decompressed data
        uint64_t originalSize;                  // Expected original size
        uint64_t decompressedSize;              // Actual decompressed size
        bool success;                           // Operation success status
        std::string errorMessage;               // Error message if failed

        /**
         * @brief Get human-readable summary
         * @return Formatted summary string
         */
        std::string getSummary() const;

        /**
         * @brief Format size to human-readable string
         * @param bytes Size in bytes
         * @return Formatted size string
         */
        static std::string formatSize(uint64_t bytes);
    };

    /**
     * @brief Compression statistics
     */
    struct CompressionStats {
        uint64_t totalOriginalSize;             // Total original bytes
        uint64_t totalCompressedSize;           // Total compressed bytes
        uint64_t filesProcessed;                // Number of files processed
        uint64_t directoriesProcessed;          // Number of directories processed
        double averageCompressionRatio;         // Average compression ratio
        uint64_t timeMs;                        // Processing time in milliseconds

        /**
         * @brief Get human-readable summary
         * @return Formatted summary string
         */
        std::string getSummary() const;

        /**
         * @brief Get savings percentage
         * @return Percentage saved (negative if grew)
         */
        double getSavingsPercentage() const;

        /**
         * @brief Format size to human-readable string
         * @param bytes Size in bytes
         * @return Formatted size string
         */
        static std::string formatSize(uint64_t bytes);
    };

    /**
     * @brief Compression engine using zlib (DEFLATE algorithm)
     *
     * This class provides compression and decompression using the DEFLATE
     * algorithm via zlib, which offers a good balance between compression
     * ratio and speed.
     */
    class CompressionEngine {
    private:
        int m_compressionLevel;                 // Current compression level
        uint32_t m_windowBits;                  // Window bits for DEFLATE
        bool m_initialized;                     // Initialization state

        // Streaming buffers
        static constexpr size_t CHUNK_SIZE = 64 * 1024;  // 64KB chunks

    public:
        /**
         * @brief Default constructor
         * @param level Compression level (0-9, default: 6)
         */
        explicit CompressionEngine(int level = CompressionLevel::DEFAULT);

        /**
         * @brief Destructor
         */
        ~CompressionEngine();

        /**
         * @brief Initialize the compression engine
         * @return true if initialization successful
         */
        bool initialize();

        /**
         * @brief Check if engine is initialized
         * @return true if initialized
         */
        bool isInitialized() const;

        /**
         * @brief Set compression level
         * @param level Compression level (0-9)
         */
        void setCompressionLevel(int level);

        /**
         * @brief Get current compression level
         * @return Compression level
         */
        int getCompressionLevel() const;

        /**
         * @brief Compress data using DEFLATE
         * @param data Data to compress
         * @return Compression result
         */
        CompressionResult compress(const std::vector<uint8_t>& data);

        /**
         * @brief Compress data from file
         * @param filepath Path to input file
         * @return Compression result
         */
        CompressionResult compressFile(const std::string& filepath);

        /**
         * @brief Decompress data using INFLATE
         * @param compressedData Compressed data
         * @param expectedSize Expected decompressed size (0 for unknown)
         * @return Decompression result
         */
        DecompressionResult decompress(
            const std::vector<uint8_t>& compressedData,
            uint64_t expectedSize = 0
        );

        /**
         * @brief Decompress to file
         * @param compressedData Compressed data
         * @param outputPath Path to output file
         * @param expectedSize Expected decompressed size
         * @return true if successful
         */
        bool decompressToFile(
            const std::vector<uint8_t>& compressedData,
            const std::string& outputPath,
            uint64_t expectedSize = 0
        );

        /**
         * @brief Compress data with streaming (memory efficient)
         * @param inputCallback Callback to read input data
         * @param outputCallback Callback to write compressed data
         * @return Compression result
         */
        template<typename InputCallback, typename OutputCallback>
        CompressionResult compressStreaming(
            InputCallback inputCallback,
            OutputCallback outputCallback
        );

        /**
         * @brief Decompress data with streaming (memory efficient)
         * @param inputCallback Callback to read compressed data
         * @param outputCallback Callback to write decompressed data
         * @param expectedSize Expected decompressed size
         * @return Decompression result
         */
        template<typename InputCallback, typename OutputCallback>
        DecompressionResult decompressStreaming(
            InputCallback inputCallback,
            OutputCallback outputCallback,
            uint64_t expectedSize = 0
        );

        /**
         * @brief Compress directory recursively
         * @param inputDir Input directory path
         * @param outputFile Output file path
         * @return Compression statistics
         */
        CompressionStats compressDirectory(
            const std::string& inputDir,
            const std::string& outputFile
        );

        /**
         * @brief Get compression level name
         * @param level Compression level
         * @return Level name string
         */
        static std::string getLevelName(int level);

        /**
         * @brief Check if data appears to be compressed
         * @param data Data to check
         * @return true if likely compressed (DEFLATE format)
         */
        static bool isCompressed(const std::vector<uint8_t>& data);

        /**
         * @brief Get algorithm information
         * @return Algorithm description
         */
        static std::string getAlgorithmInfo();

        /**
         * @brief Estimate compression ratio for data
         * @param data Data to analyze
         * @return Estimated compression ratio
         */
        static double estimateCompressionRatio(const std::vector<uint8_t>& data);

        /**
         * @brief Get optimal compression level for data type
         * @param dataType File type identifier
         * @return Recommended compression level
         */
        static int getOptimalLevel(uint32_t dataType);
    };

    // Template implementations

    template<typename InputCallback, typename OutputCallback>
    CompressionResult CompressionEngine::compressStreaming(
        InputCallback inputCallback,
        OutputCallback outputCallback
    ) {
        CompressionResult result;
        result.success = false;
        result.originalSize = 0;
        result.compressedSize = 0;
        result.compressionRatio = 0.0;

        // Initialize zlib
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

        std::vector<uint8_t> inBuffer(CHUNK_SIZE);
        std::vector<uint8_t> outBuffer(CHUNK_SIZE);

        size_t bytesRead = 0;

        try {
            // Compress in chunks
            do {
                bytesRead = inputCallback(inBuffer.data(), CHUNK_SIZE);
                strm.next_in = inBuffer.data();
                strm.avail_in = static_cast<uInt>(bytesRead);
                result.originalSize += bytesRead;

                if (bytesRead == 0) {
                    strm.next_in = Z_NULL;
                    strm.avail_in = 0;
                }

                do {
                    strm.next_out = outBuffer.data();
                    strm.avail_out = static_cast<uInt>(outBuffer.size());

                    ret = deflate(&strm, bytesRead == 0 ? Z_FINISH : Z_NO_FLUSH);

                    if (ret == Z_STREAM_ERROR) {
                        result.errorMessage = "Compression stream error";
                        deflateEnd(&strm);
                        return result;
                    }

                    size_t have = outBuffer.size() - strm.avail_out;
                    if (have > 0) {
                        outputCallback(outBuffer.data(), have);
                        result.compressedSize += have;
                    }
                } while (strm.avail_out == 0);

            } while (bytesRead > 0);

            deflateEnd(&strm);

            result.success = true;
            if (result.originalSize > 0) {
                result.compressionRatio = (100.0 * result.compressedSize) / result.originalSize;
            }

        } catch (const std::exception& e) {
            deflateEnd(&strm);
            result.errorMessage = e.what();
        }

        return result;
    }

    template<typename InputCallback, typename OutputCallback>
    DecompressionResult CompressionEngine::decompressStreaming(
        InputCallback inputCallback,
        OutputCallback outputCallback,
        uint64_t expectedSize
    ) {
        DecompressionResult result;
        result.success = false;
        result.originalSize = expectedSize;
        result.decompressedSize = 0;

        // Initialize zlib
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;

        int ret = inflateInit2(&strm, m_windowBits);

        if (ret != Z_OK) {
            result.errorMessage = "Failed to initialize decompression";
            return result;
        }

        std::vector<uint8_t> inBuffer(CHUNK_SIZE);
        std::vector<uint8_t> outBuffer(CHUNK_SIZE);

        try {
            // Decompress in chunks
            do {
                size_t bytesRead = inputCallback(inBuffer.data(), CHUNK_SIZE);
                if (bytesRead == 0 && expectedSize > 0 && result.decompressedSize >= expectedSize) {
                    break;
                }

                strm.next_in = inBuffer.data();
                strm.avail_in = static_cast<uInt>(bytesRead);

                do {
                    strm.next_out = outBuffer.data();
                    strm.avail_out = static_cast<uInt>(outBuffer.size());

                    ret = inflate(&strm, Z_NO_FLUSH);

                    if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR) {
                        result.errorMessage = "Decompression stream error";
                        inflateEnd(&strm);
                        return result;
                    }

                    size_t have = outBuffer.size() - strm.avail_out;
                    if (have > 0) {
                        outputCallback(outBuffer.data(), have);
                        result.decompressedSize += have;
                    }

                } while (strm.avail_out == 0);

            } while (ret != Z_STREAM_END);

            inflateEnd(&strm);

            // Verify size if expected
            if (expectedSize > 0 && result.decompressedSize != expectedSize) {
                result.errorMessage = "Decompressed size mismatch";
                return result;
            }

            result.success = true;

        } catch (const std::exception& e) {
            inflateEnd(&strm);
            result.errorMessage = e.what();
        }

        return result;
    }

} // namespace VaultArchive

#endif // COMPRESSIONENGINE_HPP
