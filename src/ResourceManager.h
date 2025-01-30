#pragma once

#include <nctl/Array.h>
#include <nctl/HashMap.h>
#include <nctl/String.h>

namespace ncine {
	class Texture;
	class AudioBuffer;
}

namespace nc = ncine;

class ResourceManager
{
  public:
	ResourceManager();
	~ResourceManager();

	void releaseAll();

	nc::Texture *retrieveTexture(const char *path);
	nc::AudioBuffer *retrieveAudioBuffer(const char *path);

  private:
	nctl::HashMap<nctl::String, nctl::UniquePtr<nc::Texture>> textures_;
	nctl::HashMap<nctl::String, nctl::UniquePtr<nc::AudioBuffer>> audioBuffers_;
};

// Meyers' Singleton
extern ResourceManager &resourceManager();
