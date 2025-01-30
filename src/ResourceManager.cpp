#include "ResourceManager.h"
#include <ncine/FileSystem.h>
#include <ncine/Texture.h>
#include <ncine/AudioBuffer.h>

ResourceManager &resourceManager()
{
	static ResourceManager instance;
	return instance;
}

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

ResourceManager::ResourceManager()
    : textures_(256), audioBuffers_(256)
{
}

ResourceManager::~ResourceManager()
{
	releaseAll();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void ResourceManager::releaseAll()
{
	textures_.clear();
	audioBuffers_.clear();
}

nc::Texture *ResourceManager::retrieveTexture(const char *path)
{
	nctl::UniquePtr<nc::Texture> *textureEntry = nullptr;
	nc::Texture *retrievedTexture = nullptr;
	const nctl::String absolutePath_ = nc::fs::joinPath(nc::fs::dataPath(), path);

	textureEntry = textures_.find(absolutePath_.data());
	if (textureEntry != nullptr)
	{
		retrievedTexture = textureEntry->get();
		FATAL_ASSERT(retrievedTexture != nullptr);
	}
	else
	{
		nctl::UniquePtr<nc::Texture> newTexture = nctl::makeUnique<nc::Texture>(absolutePath_.data());
		if (newTexture->dataSize() == 0)
			LOGW_X("Cannot load texture: %s", absolutePath_.data());
		else
		{
			if (textures_.loadFactor() >= 0.8f)
				textures_.rehash(textures_.capacity() * 2);

			retrievedTexture = newTexture.get(); // get pointer before moving inside the hashmap
			textures_.insert(absolutePath_.data(), nctl::move(newTexture));
		}
	}

	return retrievedTexture;
}

nc::AudioBuffer *ResourceManager::retrieveAudioBuffer(const char *path)
{
	nctl::UniquePtr<nc::AudioBuffer> *audioBufferEntry = nullptr;
	nc::AudioBuffer *retrievedAudioBuffer = nullptr;
	const nctl::String absolutePath_ = nc::fs::joinPath(nc::fs::dataPath(), path);

	audioBufferEntry = audioBuffers_.find(absolutePath_.data());
	if (audioBufferEntry != nullptr)
	{
		retrievedAudioBuffer = audioBufferEntry->get();
		FATAL_ASSERT(retrievedAudioBuffer != nullptr);
	}
	else
	{
		nctl::UniquePtr<nc::AudioBuffer> newAudioBuffer = nctl::makeUnique<nc::AudioBuffer>(absolutePath_.data());
		if (newAudioBuffer->bufferSize() == 0)
			LOGW_X("Cannot load audio buffer: %s", absolutePath_.data());
		else
		{
			if (audioBuffers_.loadFactor() >= 0.8f)
				audioBuffers_.rehash(audioBuffers_.capacity() * 2);

			retrievedAudioBuffer = newAudioBuffer.get(); // get pointer before moving inside the hashmap
			audioBuffers_.insert(absolutePath_.data(), nctl::move(newAudioBuffer));
		}
	}

	return retrievedAudioBuffer;
}
