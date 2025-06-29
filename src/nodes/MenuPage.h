#pragma once

#include "LogicNode.h"
#include <nctl/StaticArray.h>
#include <nctl/BitSet.h>

namespace ncine {
	class Font;
	class TextNode;
	class AudioBufferPlayer;
}
class Menu;

namespace nc = ncine;

class MenuPage : public LogicNode
{
  public:
	static const unsigned int MaxNumEntries = 8;

	enum class EventType
	{
		SELECT,
		LEFT,
		RIGHT,
		TEXT
	};

	struct EntryEvent
	{
		EntryEvent(EventType tt, nctl::String &text, void *data);

		const EventType type;
		nctl::String &entryText;
		void *entryData;
		bool shouldUpdateEntryText;
	};

	struct PageEntry
	{
		using EventFunctionT = void(EntryEvent &event);
		using EventReplyBitsT = nctl::BitSet<unsigned char>;

		static const unsigned char SelectBitPos = static_cast<unsigned char>(EventType::SELECT);
		static const unsigned char LeftBitPos = static_cast<unsigned char>(EventType::LEFT);
		static const unsigned char RightBitPos = static_cast<unsigned char>(EventType::RIGHT);
		static const unsigned char TextBitPos = static_cast<unsigned char>(EventType::TEXT);

		nctl::String text;
		void *data = nullptr;
		EventFunctionT *eventFunc = nullptr;
		EventReplyBitsT eventReplyBits;

		PageEntry() = default;
		explicit PageEntry(const char *t);
		PageEntry(const char *t, void *d);
		PageEntry(const char *t, void *d, EventFunctionT *f, EventReplyBitsT b);
	};

	struct PageConfig
	{
		using BackFunctionT = void();

		nctl::String title;
		BackFunctionT *backFunc = nullptr;
		nctl::StaticArray<PageEntry, MaxNumEntries> entries;
	};

	MenuPage(SceneNode *parent, nctl::String name);

	void setup(const PageConfig &config);
	void updateEntryText(unsigned int entryIndex);
	void updateAllEntriesText();
	void enableActions(bool value);
	void setSfxVolume(float gain);

	void onTick(float deltaTime) override;
	void drawGui();

  private:
	PageConfig config_;
	nctl::UniquePtr<nc::Font> menuFont_;
	nctl::UniquePtr<nc::TextNode> titleTextNode_;
	nctl::StaticArray<nctl::UniquePtr<nc::TextNode>, MaxNumEntries> entryTextNodes_;

	nctl::UniquePtr<nc::AudioBufferPlayer> clickSoundPlayer_;
	nctl::UniquePtr<nc::AudioBufferPlayer> selectSoundPlayer_;
	nctl::UniquePtr<nc::AudioBufferPlayer> backSoundPlayer_;

	unsigned int hoveredEntry_;
	bool actionsEnabled_;

	void actionUp(bool withSound);
	void actionDown(bool withSound);
	void actionEntry(EventType type, unsigned int entryIndex);
	void actionBack();

	bool isHoverable(unsigned int entryNum);
	void setHovered(unsigned int entryNum, bool hovered);
};
