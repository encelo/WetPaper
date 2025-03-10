#include <ncine/config.h>
#if !NCINE_WITH_PNG
	#error nCine must have libpng support enabled for this application to work
#endif
#if !NCINE_WITH_VORBIS
	#error nCine must have Ogg Vorbis support enabled for this application to work
#endif

#include "main.h"
#include "Config.h"
#include "ResourceManager.h"

#include <ncine/ILogger.h>
#include <ncine/Application.h>
#include <ncine/AppConfiguration.h>
#include <ncine/IFile.h>

#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
	#include <nctl/CString.h>
	#include <nctl/StaticString.h>
	#include <ncine/FileSystem.h>
#endif

namespace {

	struct WindowState
	{
		nc::Recti rect;
	};

	bool saveWindowState(const WindowState &ws)
	{
		nctl::UniquePtr<nc::IFile> file = nc::IFile::createFileHandle(Cfg::WindowStateFilename);
		file->open(nc::IFile::OpenMode::WRITE | nc::IFile::OpenMode::BINARY);
		if (file->isOpened() == false)
		{
			LOGW_X("Cannot open window state file: %s", Cfg::WindowStateFilename);
			return false;
		}

		file->write(&ws, sizeof(WindowState));
		file->close();
		return true;
	}

	WindowState loadWindowState()
	{
		WindowState ws = {};
		ws.rect = nc::Recti(0, 0, Cfg::Game::Resolution.x, Cfg::Game::Resolution.y); // default value

		nctl::UniquePtr<nc::IFile> file = nc::IFile::createFileHandle(Cfg::WindowStateFilename);
		file->open(nc::IFile::OpenMode::READ | nc::IFile::OpenMode::BINARY);
		if (file->isOpened() == false)
		{
			LOGW_X("Cannot open window state file: %s", Cfg::WindowStateFilename);
			return ws;
		}

		const unsigned long fileSize = file->size();
		ASSERT(fileSize == sizeof(WindowState));
		file->read(&ws, sizeof(WindowState));
		file->close();
		return ws;
	}

	bool showInterface = true;

}

nctl::UniquePtr<nc::IAppEventHandler> createAppEventHandler()
{
	return nctl::makeUnique<MyEventHandler>();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void MyEventHandler::onPreInit(nc::AppConfiguration &config)
{
#if defined(__ANDROID__)
	config.dataPath() = "asset::";
#elif defined(__EMSCRIPTEN__)
	config.dataPath() = "/";
#else
	#ifdef NCPROJECT_DEFAULT_DATA_DIR
	config.dataPath() = NCPROJECT_DEFAULT_DATA_DIR;
	#else
	config.dataPath() = "data/";
	#endif
#endif

	WindowState ws = loadWindowState();

	config.windowTitle = "Wet Paper";
	config.windowIconFilename = "icon48.png";

	config.windowPosition = nc::Vector2i(ws.rect.x, ws.rect.y);
#ifndef NCPROJECT_DEBUG
	config.resizable = false;
	config.resolution = Cfg::Game::Resolution;
	config.consoleLogLevel = nc::ILogger::LogLevel::OFF;
#else
	config.resizable = true;
	config.resolution = nc::Vector2i(ws.rect.w, ws.rect.h);
	config.consoleLogLevel = nc::ILogger::LogLevel::INFO;
#endif
}

void MyEventHandler::onInit()
{
#ifdef NCPROJECT_DEBUG
	nc::theApplication().setAutoSuspension(false);
#endif

	nc::SceneNode &rootNode = nc::theApplication().rootNode();
	menu_ = nctl::makeUnique<Menu>(&rootNode, "MENU", this);
}

void MyEventHandler::onShutdown()
{
	resourceManager().releaseAll();

	WindowState ws = {};
	ws.rect.x = nc::theApplication().gfxDevice().windowPositionX();
	ws.rect.y = nc::theApplication().gfxDevice().windowPositionY();
	ws.rect.w = nc::theApplication().gfxDevice().resolution().x;
	ws.rect.h = nc::theApplication().gfxDevice().resolution().y;

	saveWindowState(ws);
}

void MyEventHandler::onFrameStart()
{
	if (requestMenuTransition_)
	{
		showMenu();
		requestMenuTransition_ = false;
	}
	else if (requestGameTransition_)
	{
		showGame();
		requestGameTransition_ = false;
	}

#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	if (showInterface)
	{
		const float scalingFactor = nc::theApplication().gfxDevice().windowScalingFactor();
		if (ImGui::GetIO().FontGlobalScale != scalingFactor)
		{
			ImGui::GetIO().FontGlobalScale = scalingFactor;
			ImGui::GetStyle() = ImGuiStyle();
			ImGui::GetStyle().ScaleAllSizes(scalingFactor);
		}

		if (ImGui::Begin("Wet Paper", &showInterface))
		{
			const float scaling = nc::theApplication().gfxDevice().windowScalingFactor();
			if (scaling != 1.0f)
				ImGui::Text("Window scaling factor: %.2f", scaling);

			const float deltaTime = nc::theApplication().frameTime();
			ImGui::Text("Delta time: %0.3f ms (%0.1f FPS)", deltaTime * 1000.0f, 1.0f / deltaTime);

			if (menu_ != nullptr)
				menu_->drawGui();
			if (game_ != nullptr)
				game_->drawGui();

			if (ImGui::Button("QUIT"))
				nc::theApplication().quit();

			ImGui::End();
		}
	}
#endif
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.mod & nc::KeyMod::CTRL && event.sym == nc::KeySym::H)
		showInterface = !showInterface;
	else if (event.mod & nc::KeyMod::CTRL && event.sym == nc::KeySym::Q)
		nc::theApplication().quit();
}

void MyEventHandler::onChangeScalingFactor(float factor)
{
	if (menu_ != nullptr)
		menu_->setScale(factor);
	if (game_ != nullptr)
		game_->setScale(factor);
}

void MyEventHandler::requestMenu()
{
	requestMenuTransition_ = true;
}

void MyEventHandler::requestGame()
{
	requestGameTransition_ = true;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void MyEventHandler::showGame()
{
	menu_.reset(nullptr);

	nc::SceneNode &rootNode = nc::theApplication().rootNode();
	game_ = nctl::makeUnique<Game>(&rootNode, "GAME", this);
}

void MyEventHandler::showMenu()
{
	game_.reset(nullptr);

	nc::SceneNode &rootNode = nc::theApplication().rootNode();
	menu_ = nctl::makeUnique<Menu>(&rootNode, "MENU", this);
}
