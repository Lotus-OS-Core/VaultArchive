/**
 * @file CryptoEngine.hpp
 * @brief Cryptographic operations for VaultArchive encryption and hashing
 * @author LotusOS Core
 * @version 1.0.0
 */

#ifndef CRYPTOENGINE_HPP
#define CRYPTOENGINE_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <array>

namespace VaultArchive {

    /**
     * @brief Cryptographic engine for encryption, decryption, and hashing
     *
     * This class provides a wrapper around OpenSSL cryptographic functions
     * including AES-256-CBC encryption, PBKDF2 key derivation, and SHA-256 hashing.
     */
    class CryptoEngine {
    public:
        /**
         * @brief Encryption algorithm constants
         */
        static constexpr size_t AES_KEY_SIZE = 32;        // 256 bits
        static constexpr size_t AES_BLOCK_SIZE = 16;      // 128 bits
        static constexpr size_t SALT_SIZE = 32;           // 256 bits
        static constexpr size_t IV_SIZE = 16;             // 128 bits
        static constexpr int PBKDF2_ITERATIONS = 100000;  // OWASP recommended minimum
        static constexpr size_t HASH_SIZE = 32;           // SHA-256 output size

        /**
         * @brief Result structure for encryption operations
         */
        struct EncryptionResult {
            std::vector<uint8_t> ciphertext;   // Encrypted data
            std::vector<uint8_t> tag;          // Authentication tag (for AEAD)
        };

        /**
         * @brief Result structure for key derivation
         */
        struct KeyDerivationResult {
            std::vector<uint8_t> key;          // Derived key
            std::vector<uint8_t> salt;         // Used salt
        };

    private:
        std::vector<uint8_t> m_key;             // Current encryption key
        std::vector<uint8_t> m_iv;              // Current IV
        bool m_initialized;                     // Initialization state

    public:
        /**
         * @brief Default constructor
         */
        CryptoEngine();

        /**
         * @brief Destructor
         */
        ~CryptoEngine();

        /**
         * @brief Initialize with key and IV
         * @param key Encryption key (32 bytes for AES-256)
         * @param iv Initialization vector (16 bytes)
         */
        void initialize(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

        /**
         * @brief Initialize with password (derives key internally)
         * @param password User password
         * @param salt Salt for key derivation
         */
        void initializeFromPassword(const std::string& password, const std::vector<uint8_t>& salt);

        /**
         * @brief Check if engine is initialized
         * @return true if initialized
         */
        bool isInitialized() const;

        /**
         * @brief Clear sensitive data from memory
         */
        void clear();

        /**
         * @brief Generate a random salt
         * @param size Salt size in bytes (default: SALT_SIZE)
         * @return Random salt vector
         */
        static std::vector<uint8_t> generateSalt(size_t size = SALT_SIZE);

        /**
         * @brief Generate a random IV
         * @return Random IV vector
         */
        static std::vector<uint8_t> generateIV();

        /**
         * @brief Generate cryptographically secure random bytes
         * @param size Number of bytes to generate
         * @return Random byte vector
         */
        static std::vector<uint8_t> generateRandom(size_t size);

        /**
         * @brief Derive encryption key from password using PBKDF2
         * @param password User password
         * @param salt Salt for key derivation
         * @param iterations Number of PBKDF2 iterations
         * @param keySize Desired key size in bytes
         * @return Derived key
         */
        static std::vector<uint8_t> deriveKey(
            const std::string& password,
            const std::vector<uint8_t>& salt,
            int iterations = PBKDF2_ITERATIONS,
            size_t keySize = AES_KEY_SIZE
        );

        /**
         * @brief Encrypt data using AES-256-CBC
         * @param plaintext Data to encrypt
         * @return Encrypted data with PKCS#7 padding
         */
        std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext);

        /**
         * @brief Decrypt data using AES-256-CBC
         * @param ciphertext Data to decrypt
         * @return Decrypted data (original plaintext)
         * @throws std::runtime_error if decryption fails
         */
        std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext);

        /**
         * @brief Encrypt data with authentication (AES-256-GCM)
         * @param plaintext Data to encrypt
         * @return Encryption result containing ciphertext and tag
         */
        EncryptionResult encryptAuthenticated(const std::vector<uint8_t>& plaintext);

        /**
         * @brief Decrypt authenticated data (AES-256-GCM)
         * @param ciphertext Encrypted data
         * @param tag Authentication tag
         * @return Decrypted data
         * @throws std::runtime_error if authentication fails
         */
        std::vector<uint8_t> decryptAuthenticated(
            const std::vector<uint8_t>& ciphertext,
            const std::vector<uint8_t>& tag
        );

        /**
         * @brief Calculate SHA-256 hash
         * @param data Data to hash
         * @return SHA-256 hash (32 bytes)
         */
        static std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);

        /**
         * @brief Calculate SHA-256 hash of file
         * @param filepath Path to file
         * @return SHA-256 hash (32 bytes)
         * @throws std::runtime_error if file cannot be read
         */
        static std::vector<uint8_t> sha256File(const std::string& filepath);

        /**
         * @brief Calculate HMAC-SHA256
         * @param data Data to hash
         * @param key HMAC key
         * @return HMAC-SHA256 (32 bytes)
         */
        static std::vector<uint8_t> hmacSha256(
            const std::vector<uint8_t>& data,
            const std::vector<uint8_t>& key
        );

        /**
         * @brief Verify data integrity using stored checksum
         * @param data Data to verify
         * @param storedChecksum Expected checksum
         * @return true if checksum matches
         */
        static bool verifyChecksum(
            const std::vector<uint8_t>& data,
            const std::vector<uint8_t>& storedChecksum
        );

        /**
         * @brief Get key derivation parameters
         * @return String describing current KDF settings
         */
        static std::string getKdfInfo();

        /**
         * @brief Securely wipe memory buffer
         * @param buffer Buffer to wipe
         */
        static void secureWipe(std::vector<uint8_t>& buffer);

        /**
         * @brief Convert bytes to hex string
         * @param data Byte vector
         * @return Hexadecimal string
         */
        static std::string bytesToHex(const std::vector<uint8_t>& data);

        /**
         * @brief Convert hex string to bytes
         * @param hex Hexadecimal string
         * @return Byte vector
         */
        static std::vector<uint8_t> hexToBytes(const std::string& hex);
    };

} // namespace VaultArchive

#endif // CRYPTOENGINE_HPP
