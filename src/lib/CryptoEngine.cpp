/**
 * @file CryptoEngine.cpp
 * @brief Cryptographic operations implementation
 * @author LotusOS Core
 * @version 1.0.0
 */

#include "CryptoEngine.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cstring>

namespace VaultArchive {

    // ======================
    // CryptoEngine Implementation
    // ======================

    CryptoEngine::CryptoEngine() : m_initialized(false) {
    }

    CryptoEngine::~CryptoEngine() {
        clear();
    }

    void CryptoEngine::initialize(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv) {
        if (key.size() != AES_KEY_SIZE) {
            throw std::runtime_error("Invalid key size for AES-256");
        }
        if (iv.size() != IV_SIZE) {
            throw std::runtime_error("Invalid IV size for AES");
        }

        m_key = key;
        m_iv = iv;
        m_initialized = true;
    }

    void CryptoEngine::initializeFromPassword(const std::string& password, const std::vector<uint8_t>& salt) {
        m_key = deriveKey(password, salt);
        m_iv = generateIV();
        m_initialized = true;
    }

    bool CryptoEngine::isInitialized() const {
        return m_initialized && !m_key.empty() && !m_iv.empty();
    }

    void CryptoEngine::clear() {
        secureWipe(m_key);
        secureWipe(m_iv);
        m_key.clear();
        m_iv.clear();
        m_initialized = false;
    }

    std::vector<uint8_t> CryptoEngine::generateSalt(size_t size) {
        std::vector<uint8_t> salt(size);
        if (RAND_bytes(salt.data(), static_cast<int>(size)) != 1) {
            throw std::runtime_error("Failed to generate random salt");
        }
        return salt;
    }

    std::vector<uint8_t> CryptoEngine::generateIV() {
        std::vector<uint8_t> iv(IV_SIZE);
        if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
            throw std::runtime_error("Failed to generate random IV");
        }
        return iv;
    }

    std::vector<uint8_t> CryptoEngine::generateRandom(size_t size) {
        std::vector<uint8_t> data(size);
        if (RAND_bytes(data.data(), static_cast<int>(size)) != 1) {
            throw std::runtime_error("Failed to generate random data");
        }
        return data;
    }

    std::vector<uint8_t> CryptoEngine::deriveKey(
        const std::string& password,
        const std::vector<uint8_t>& salt,
        int iterations,
        size_t keySize
    ) {
        if (password.empty()) {
            throw std::runtime_error("Password cannot be empty for key derivation");
        }

        std::vector<uint8_t> key(keySize);

        PKCS5_PBKDF2_HMAC(
            password.c_str(),
            static_cast<int>(password.length()),
            salt.data(),
            static_cast<int>(salt.size()),
            iterations,
            EVP_sha256(),
            static_cast<int>(keySize),
            key.data()
        );

        return key;
    }

    std::vector<uint8_t> CryptoEngine::encrypt(const std::vector<uint8_t>& plaintext) {
        if (!isInitialized()) {
            throw std::runtime_error("CryptoEngine not initialized");
        }

        std::vector<uint8_t> ciphertext;
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        try {
            // Initialize encryption
            if (EVP_EncryptInit_ex(
                ctx,
                EVP_aes_256_cbc(),
                nullptr,
                m_key.data(),
                m_iv.data()
            ) != 1) {
                throw std::runtime_error("Failed to initialize encryption");
            }

            // Calculate required buffer size (plaintext + padding)
            int blockSize = EVP_CIPHER_CTX_block_size(ctx);
            ciphertext.resize(plaintext.size() + blockSize);

            int outLen1 = 0;
            int outLen2 = 0;

            // Encrypt plaintext
            if (EVP_EncryptUpdate(
                ctx,
                ciphertext.data(),
                &outLen1,
                plaintext.data(),
                static_cast<int>(plaintext.size())
            ) != 1) {
                throw std::runtime_error("Encryption update failed");
            }

            // Finalize encryption (add padding)
            if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + outLen1, &outLen2) != 1) {
                throw std::runtime_error("Encryption finalization failed");
            }

            ciphertext.resize(outLen1 + outLen2);

        } catch (...) {
            EVP_CIPHER_CTX_free(ctx);
            throw;
        }

        EVP_CIPHER_CTX_free(ctx);
        return ciphertext;
    }

    std::vector<uint8_t> CryptoEngine::decrypt(const std::vector<uint8_t>& ciphertext) {
        if (!isInitialized()) {
            throw std::runtime_error("CryptoEngine not initialized");
        }

        std::vector<uint8_t> plaintext;
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        try {
            // Initialize decryption
            if (EVP_DecryptInit_ex(
                ctx,
                EVP_aes_256_cbc(),
                nullptr,
                m_key.data(),
                m_iv.data()
            ) != 1) {
                throw std::runtime_error("Failed to initialize decryption");
            }

            // Remove padding by allocating exact size
            int blockSize = EVP_CIPHER_CTX_block_size(ctx);
            plaintext.resize(ciphertext.size());

            int outLen1 = 0;
            int outLen2 = 0;

            // Decrypt ciphertext
            if (EVP_DecryptUpdate(
                ctx,
                plaintext.data(),
                &outLen1,
                ciphertext.data(),
                static_cast<int>(ciphertext.size())
            ) != 1) {
                throw std::runtime_error("Decryption update failed (wrong password?)");
            }

            // Finalize decryption (remove padding)
            if (EVP_DecryptFinal_ex(ctx, plaintext.data() + outLen1, &outLen2) != 1) {
                throw std::runtime_error("Decryption finalization failed (corrupted data or wrong password)");
            }

            plaintext.resize(outLen1 + outLen2);

        } catch (const std::runtime_error&) {
            EVP_CIPHER_CTX_free(ctx);
            throw;
        } catch (...) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Unknown decryption error");
        }

        EVP_CIPHER_CTX_free(ctx);
        return plaintext;
    }

    CryptoEngine::EncryptionResult CryptoEngine::encryptAuthenticated(const std::vector<uint8_t>& plaintext) {
        EncryptionResult result;

        if (!isInitialized()) {
            throw std::runtime_error("CryptoEngine not initialized");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        try {
            // Initialize authenticated encryption
            if (EVP_EncryptInit_ex(
                ctx,
                EVP_aes_256_gcm(),
                nullptr,
                m_key.data(),
                m_iv.data()
            ) != 1) {
                throw std::runtime_error("Failed to initialize authenticated encryption");
            }

            // Calculate required buffer size
            int blockSize = EVP_CIPHER_CTX_block_size(ctx);
            result.ciphertext.resize(plaintext.size() + blockSize);

            int outLen1 = 0;
            int outLen2 = 0;

            // Encrypt plaintext
            if (EVP_EncryptUpdate(
                ctx,
                result.ciphertext.data(),
                &outLen1,
                plaintext.data(),
                static_cast<int>(plaintext.size())
            ) != 1) {
                throw std::runtime_error("Authenticated encryption update failed");
            }

            // Finalize encryption
            if (EVP_EncryptFinal_ex(ctx, result.ciphertext.data() + outLen1, &outLen2) != 1) {
                throw std::runtime_error("Authenticated encryption finalization failed");
            }

            result.ciphertext.resize(outLen1 + outLen2);

            // Get authentication tag
            result.tag.resize(16);  // GCM tag is 16 bytes
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, result.tag.data()) != 1) {
                throw std::runtime_error("Failed to get authentication tag");
            }

        } catch (...) {
            EVP_CIPHER_CTX_free(ctx);
            throw;
        }

        EVP_CIPHER_CTX_free(ctx);
        return result;
    }

    std::vector<uint8_t> CryptoEngine::decryptAuthenticated(
        const std::vector<uint8_t>& ciphertext,
        const std::vector<uint8_t>& tag
    ) {
        if (!isInitialized()) {
            throw std::runtime_error("CryptoEngine not initialized");
        }

        std::vector<uint8_t> plaintext;
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        try {
            // Set expected tag before initialization
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(tag.size()),
                const_cast<uint8_t*>(tag.data())) != 1) {
                throw std::runtime_error("Failed to set authentication tag");
            }

            // Initialize decryption
            if (EVP_DecryptInit_ex(
                ctx,
                EVP_aes_256_gcm(),
                nullptr,
                m_key.data(),
                m_iv.data()
            ) != 1) {
                throw std::runtime_error("Failed to initialize authenticated decryption");
            }

            // Allocate output buffer
            plaintext.resize(ciphertext.size());

            int outLen1 = 0;
            int outLen2 = 0;

            // Decrypt ciphertext
            if (EVP_DecryptUpdate(
                ctx,
                plaintext.data(),
                &outLen1,
                ciphertext.data(),
                static_cast<int>(ciphertext.size())
            ) != 1) {
                throw std::runtime_error("Authenticated decryption update failed");
            }

            // Finalize and verify tag
            int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + outLen1, &outLen2);
            if (ret != 1) {
                throw std::runtime_error("Authentication failed - data has been tampered with or wrong password");
            }

            plaintext.resize(outLen1 + outLen2);

        } catch (const std::runtime_error&) {
            EVP_CIPHER_CTX_free(ctx);
            throw;
        } catch (...) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Unknown authenticated decryption error");
        }

        EVP_CIPHER_CTX_free(ctx);
        return plaintext;
    }

    std::vector<uint8_t> CryptoEngine::sha256(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> hash(HASH_SIZE);

        SHA256(
            data.data(),
            static_cast<size_t>(data.size()),
            hash.data()
        );

        return hash;
    }

    std::vector<uint8_t> CryptoEngine::sha256File(const std::string& filepath) {
        std::vector<uint8_t> hash(HASH_SIZE);

        // Use OpenSSL's SHA256 functions
        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file for hashing: " + filepath);
        }

        std::vector<char> buffer(64 * 1024);  // 64KB chunks
        while (file.good()) {
            file.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = file.gcount();
            if (bytesRead > 0) {
                SHA256_Update(&sha256, buffer.data(), static_cast<size_t>(bytesRead));
            }
        }

        file.close();
        SHA256_Final(hash.data(), &sha256);

        return hash;
    }

    std::vector<uint8_t> CryptoEngine::hmacSha256(
        const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& key
    ) {
        std::vector<uint8_t> hash(HASH_SIZE);

        HMAC(
            EVP_sha256(),
            key.data(),
            static_cast<int>(key.size()),
            data.data(),
            static_cast<size_t>(data.size()),
            hash.data(),
            nullptr
        );

        return hash;
    }

    bool CryptoEngine::verifyChecksum(
        const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& storedChecksum
    ) {
        if (storedChecksum.size() != HASH_SIZE) {
            return false;
        }

        std::vector<uint8_t> calculatedChecksum = sha256(data);
        return calculatedChecksum == storedChecksum;
    }

    std::string CryptoEngine::getKdfInfo() {
        std::ostringstream oss;
        oss << "PBKDF2-HMAC-SHA256\n";
        oss << "Iterations: " << PBKDF2_ITERATIONS << "\n";
        oss << "Key size: " << (AES_KEY_SIZE * 8) << " bits\n";
        oss << "Salt size: " << (SALT_SIZE * 8) << " bits";
        return oss.str();
    }

    void CryptoEngine::secureWipe(std::vector<uint8_t>& buffer) {
        if (buffer.empty()) {
            return;
        }

        // Use volatile pointer to prevent optimization
        volatile unsigned char* p = buffer.data();
        size_t size = buffer.size();

        // Overwrite with zeros
        std::memset(const_cast<unsigned char*>(p), 0, size);

        // Second pass with random data
        RAND_bytes(const_cast<unsigned char*>(p), static_cast<int>(size));

        // Final pass with zeros
        std::memset(const_cast<unsigned char*>(p), 0, size);
    }

    std::string CryptoEngine::bytesToHex(const std::vector<uint8_t>& data) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (size_t i = 0; i < data.size(); ++i) {
            oss << std::setw(2) << static_cast<int>(data[i]);
        }

        return oss.str();
    }

    std::vector<uint8_t> CryptoEngine::hexToBytes(const std::string& hex) {
        if (hex.length() % 2 != 0) {
            throw std::runtime_error("Invalid hex string: odd length");
        }

        std::vector<uint8_t> bytes;
        bytes.reserve(hex.length() / 2);

        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteStr = hex.substr(i, 2);
            unsigned int byte;
            std::istringstream iss(byteStr);
            iss >> std::hex >> byte;
            bytes.push_back(static_cast<uint8_t>(byte));
        }

        return bytes;
    }

} // namespace VaultArchive
