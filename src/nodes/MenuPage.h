#pragma once

#include "LogicNode.h"
#include <nctl/StaticArray.h>

namespace ncine {
	class Font;
	class TextNode;
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
		using EventReplyFunctionT = bool(EventType type);

		nctl::String text;
		void *data = nullptr;
		EventFunctionT *eventFunc = nullptr;
#if defined(NCPROJECT_DEBUG)
		EventReplyFunctionT *eventReplyFunc = nullptr;
#endif

		PageEntry() = default;
		explicit PageEntry(const char *t);
		PageEntry(const char *t, void *d);
		PageEntry(const char *t, void *d, EventFunctionT *f);
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

	void onTick(float deltaTime) override;
	void drawGui();

  private:
	PageConfig config_;
	nctl::UniquePtr<nc::Font> menuFont_;
	nctl::UniquePtr<nc::TextNode> titleTextNode_;
	nctl::StaticArray<nctl::UniquePtr<nc::TextNode>, MaxNumEntries> entryTextNodes_;

	unsigned int hoveredEntry_;
	bool actionsEnabled_;

	void actionUp();
	void actionDown();
	void actionEntry(EventType type, unsigned int entryIndex);
	void actionBack();

	void setHovered(unsigned int entryNum, bool hovered);
};
