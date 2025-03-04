#pragma once

#include <nctl/Array.h>
#include <nctl/HashMap.h>
#include <nctl/String.h>
#include <mutex>
#include <shared_mutex>
#include <filesystem>
#include <memory>
#include <optional>
#include <chrono>
#include <condition_variable>

namespace ncine {
    class Texture;
    class AudioBuffer;
}

namespace nc = ncine;

class ResourceManager {
public:
    // Configuration options
    struct Config {
        size_t maxTextureCacheSize = 1024;  // Maximum number of cached textures
        size_t maxAudioCacheSize = 512;     // Maximum number of cached audio buffers
        std::chrono::minutes cacheTimeout{30};  // Resource timeout duration
    };

    ResourceManager(const Config& config = Config());
    ~ResourceManager();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Core resource management
    std::optional<nc::Texture*> retrieveTexture(const char* path);
    std::optional<nc::AudioBuffer*> retrieveAudioBuffer(const char* path);

    // New features
    void releaseAll();  // Clear all resources
    bool preloadResource(const char* path, bool isTexture);  // Preload specific resource
    void setCacheTimeout(std::chrono::minutes timeout);  // Adjust cache timeout
    size_t getCacheSize() const;  // Get current cache size
    void trimCache();  // Remove expired resources

    // Resource statistics
    struct ResourceStats {
        size_t textureCount;
        size_t audioCount;
        size_t totalMemoryUsage;
        size_t hitCount;
        size_t missCount;
    };
    ResourceStats getStats() const;

private:
    struct ResourceEntry {
        std::chrono::steady_clock::time_point lastAccess;
        size_t memorySize;
        unsigned int useCount;
    };

    // Resource loading helpers
    std::unique_ptr<nc::Texture> loadTexture(const char* path);
    std::unique_ptr<nc::AudioBuffer> loadAudioBuffer(const char* path);

    // Cache management
    void updateStats(bool isHit, size_t resourceSize, bool isTexture);
    bool isCacheFull(bool isTexture) const;
    void evictOldestResource(bool isTexture);

    // Data members
    Config config_;
    mutable std::shared_mutex mutex_;  // For thread-safe access

    nctl::HashMap<nctl::String, 
                 std::pair<std::unique_ptr<nc::Texture>, ResourceEntry>> textures_;
    nctl::HashMap<nctl::String, 
                 std::pair<std::unique_ptr<nc::AudioBuffer>, ResourceEntry>> audioBuffers_;

    // Statistics
    size_t textureHits_{0};
    size_t textureMisses_{0};
    size_t audioHits_{0};
    size_t audioMisses_{0};
    size_t totalMemoryUsage_{0};
};

// Implementation
inline ResourceManager::ResourceManager(const Config& config) 
    : config_(config) {}

inline ResourceManager::~ResourceManager() {
    releaseAll();
}

inline std::optional<nc::Texture*> ResourceManager::retrieveTexture(const char* path) {
    if (!path) return std::nullopt;

    nctl::String key(path);
    std::unique_lock lock(mutex_);

    auto it = textures_.find(key);
    if (it != textures_.end()) {
        it->value().second.lastAccess = std::chrono::steady_clock::now();
        it->value().second.useCount++;
        updateStats(true, it->value().second.memorySize, true);
        return it->value().first.get();
    }

    if (isCacheFull(true)) {
        evictOldestResource(true);
    }

    auto texture = loadTexture(path);
    if (!texture) return std::nullopt;

    auto size = texture->getMemorySize(); // Assume this method exists
    ResourceEntry entry{std::chrono::steady_clock::now(), size, 1};
    auto* ptr = texture.get();
    textures_.emplace(key, std::make_pair(std::move(texture), entry));
    updateStats(false, size, true);
    return ptr;
}

// Meyers' Singleton implementation
inline ResourceManager& resourceManager() {
    static ResourceManager instance;
    return instance;
}

// Additional method implementations would go here...
