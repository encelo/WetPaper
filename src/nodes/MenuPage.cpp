#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "MenuPage.h"
#include "../Config.h"
#include "../ResourceManager.h"
#include "../InputBinder.h"
#include "../InputActions.h"

#include <nctl/CString.h>
#include <ncine/Font.h>
#include <ncine/TextNode.h>
#include <ncine/FileSystem.h>

#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
namespace {
	nctl::String auxString;
}
#endif

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

MenuPage::MenuPage(SceneNode *parent, nctl::String name)
    : LogicNode(parent, name), hoveredEntry_(0), actionsEnabled_(true)
{
	const nctl::String menuFontFntPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak50Fnt);
	menuFont_ = nctl::makeUnique<nc::Font>(menuFontFntPath_.data(), resourceManager().retrieveTexture(Cfg::Fonts::Modak50Png));

	titleTextNode_ = nctl::makeUnique<nc::TextNode>(this, menuFont_.get(), Cfg::Menu::MaxMenuEntryLength);
	titleTextNode_->setLayer(Cfg::Layers::Menu_Page);
	titleTextNode_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	titleTextNode_->setAlignment(nc::TextNode::Alignment::CENTER);
	titleTextNode_->setEnabled(false);
	titleTextNode_->setScale(1.25f);
	titleTextNode_->setColorF(0.6f, 1.0f, 0.6f, 1.0f);

	for (unsigned int i = 0; i < MaxNumEntries; i++)
	{
		nctl::UniquePtr<nc::TextNode> textNode = nctl::makeUnique<nc::TextNode>(this, menuFont_.get(), Cfg::Menu::MaxMenuEntryLength);
		textNode->setLayer(Cfg::Layers::Menu_Page);
		textNode->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
		textNode->setAlignment(nc::TextNode::Alignment::CENTER);
		textNode->setEnabled(false);

		entryTextNodes_.pushBack(nctl::move(textNode));
	}
}

MenuPage::EntryEvent::EntryEvent(EventType tt, nctl::String &text, void *data)
    : type(tt), entryText(text), entryData(data), shouldUpdateEntryText(false)
{
}

MenuPage::PageEntry::PageEntry(const char *t)
    : text(t)
{
}

MenuPage::PageEntry::PageEntry(const char *t, void *d)
    : text(t), data(d)
{
}

MenuPage::PageEntry::PageEntry(const char *t, void *d, EventFunctionT *f)
    : text(t), data(d), eventFunc(f)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void MenuPage::setup(const PageConfig &config)
{
	const unsigned int numEntries = config.entries.size();
	FATAL_ASSERT(numEntries > 0 && numEntries <= MaxNumEntries);

	// Keep a (writable) copy of the new page configuration
	config_ = config;

	float verticalPos = 0.0f;
	titleTextNode_->setEnabled(false);
	if (config.title.length() > 0)
	{
		titleTextNode_->setString(config.title);
		titleTextNode_->setEnabled(true);
		verticalPos -= titleTextNode_->height();
		titleTextNode_->setPosition(0.0f, verticalPos);
		// Add some space between the title and the entries
		verticalPos -= titleTextNode_->height() * 0.5f;
	}

	for (unsigned int i = 0; i < numEntries; i++)
	{
		PageEntry &entry = config_.entries[i];
		PageEntry::EventFunctionT *eventFunc = entry.eventFunc;
		if (eventFunc)
		{
			EntryEvent event(EventType::TEXT, entry.text, entry.data);
			entry.eventFunc(event);
		}

		nc::TextNode &textNode = *entryTextNodes_[i];
		textNode.setString(entry.text);
		textNode.setEnabled(true);
		verticalPos -= textNode.height();
		textNode.setPosition(0.0f, verticalPos);
		setHovered(i, false);
	}

	for (unsigned int i = numEntries; i < MaxNumEntries; i++)
		entryTextNodes_[i]->setEnabled(false);

	hoveredEntry_ = 0;
	setHovered(hoveredEntry_, true);
}

void MenuPage::updateEntryText(unsigned int entryIndex)
{
	const unsigned int numEntries = config_.entries.size();
	ASSERT(entryIndex < numEntries);

	PageEntry &entry = config_.entries[entryIndex];
	PageEntry::EventFunctionT *eventFunc = entry.eventFunc;
#if defined(NCPROJECT_DEBUG)
	ASSERT(entry.eventFunc != nullptr && entry.eventReplyFunc != nullptr);
	if (entry.eventReplyFunc(EventType::TEXT) == false)
		LOGW_X("Trying to update the text of entry #%u that doesn't reply to a TEXT event", entryIndex);
#endif

	if (eventFunc)
	{
		EntryEvent event(EventType::TEXT, entry.text, entry.data);
		entry.eventFunc(event);
		FATAL_ASSERT(event.shouldUpdateEntryText == false);

		nc::TextNode &textNode = *entryTextNodes_[entryIndex];
		textNode.setString(entry.text);
	}
}

void MenuPage::updateAllEntriesText()
{
	const unsigned int numEntries = config_.entries.size();
	for (unsigned int i = 0; i < numEntries; i++)
		updateEntryText(i);
}

void MenuPage::enableActions(bool value)
{
	actionsEnabled_ = value;
}

void MenuPage::onTick(float deltaTime)
{
	if (actionsEnabled_ == false)
		return;

	InputBinder &ib = inputBinder();
	const InputActions &ia = inputActions();

	if (ib.isTriggered(ia.UI_NEXT))
		actionDown();
	else if (ib.isTriggered(ia.UI_PREV))
		actionUp();
	else if (ib.isTriggered(ia.UI_LEFT))
		actionEntry(EventType::LEFT, hoveredEntry_);
	else if (ib.isTriggered(ia.UI_RIGHT))
		actionEntry(EventType::RIGHT, hoveredEntry_);
	else if (ib.isTriggered(ia.UI_ENTER))
		actionEntry(EventType::SELECT, hoveredEntry_);
	else if (ib.isTriggered(ia.UI_BACK))
		actionBack();
}

void MenuPage::drawGui()
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	if (config_.title.length() > 0)
		ImGui::Text("Page Title: %s", config_.title.data());

	ImGui::BeginDisabled(actionsEnabled_ == false);
	for (unsigned int i = 0; i < config_.entries.size(); i++)
	{
		const PageEntry &entry = config_.entries[i];
		const char *entryText = entry.text.data();
		ASSERT(entry.eventReplyFunc != nullptr);

		const bool hasEventFunctions = (entry.eventFunc != nullptr && entry.eventReplyFunc != nullptr);
		if (hasEventFunctions)
		{
			if (entry.eventReplyFunc(EventType::SELECT))
			{
				if (ImGui::Button(entryText))
					actionEntry(EventType::SELECT, i);
			}
			else if (entry.eventReplyFunc(EventType::LEFT) && entry.eventReplyFunc(EventType::RIGHT))
			{
				auxString.format("<##%s", entryText);
				if (ImGui::Button(auxString.data()))
					actionEntry(EventType::LEFT, i);

				ImGui::SameLine();
				ImGui::Text("%s", entryText);
				ImGui::SameLine();

				auxString.format(">##%s", entryText);
				if (ImGui::Button(auxString.data()))
					actionEntry(EventType::RIGHT, i);
			}
			else
			{
				// The entry only replies to the TEXT event, show it in the GUI as text anyway
				ImGui::Text("%s", entryText);
			}
		}
		else
		{
			// The entry doesn't have an event function, show it in the GUI as text anyway
			ImGui::Text("%s", entryText);
		}
	}

	PageConfig::BackFunctionT *backFunc = config_.backFunc;
	if (backFunc != nullptr)
	{
		auxString.format("Page Back##%s", config_.title.data());
		if (ImGui::Button(auxString.data()))
			backFunc();
	}
	ImGui::EndDisabled();
#endif
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void MenuPage::actionUp()
{
	if (hoveredEntry_ > 0)
	{
		setHovered(hoveredEntry_, false);
		hoveredEntry_--;
		setHovered(hoveredEntry_, true);
	}
}

void MenuPage::actionDown()
{
	const unsigned int numEntries = config_.entries.size();
	if (hoveredEntry_ < numEntries - 1)
	{
		setHovered(hoveredEntry_, false);
		hoveredEntry_++;
		setHovered(hoveredEntry_, true);
	}
}

void MenuPage::actionEntry(EventType type, unsigned int entryIndex)
{
	PageEntry &entry = config_.entries[entryIndex];
	if (entry.eventFunc == nullptr)
		return;

	EntryEvent event(type, entry.text, entry.data);
	entry.eventFunc(event);

	if (event.shouldUpdateEntryText)
		updateEntryText(entryIndex);
}

void MenuPage::actionBack()
{
	PageConfig::BackFunctionT *backFunc = config_.backFunc;
	if (backFunc != nullptr)
		backFunc();
}

void MenuPage::setHovered(unsigned int entryNum, bool hovered)
{
	const unsigned int numEntries = config_.entries.size();
	if (entryNum < numEntries)
	{
		nc::TextNode &textNode = *entryTextNodes_[entryNum];
		if (hovered)
			textNode.setColorF(1.0f, 1.0f, 1.0f, 1.0f);
		else
			textNode.setColorF(0.75f, 0.75f, 0.75f, 0.75f);
	}
}
