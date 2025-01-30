#pragma once

#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(WETPAPER_DEBUG)
	#include <ncine/imgui.h>
#endif

#include <ncine/Application.h>
#include <ncine/Color.h>
#include <ncine/Vector2.h>

namespace nc = ncine;

class DebugDraw
{
  public:
	static inline void Line(nc::Vector2f from, nc::Vector2f to)
	{
		Line(from, to, nc::Color(255, 255, 255, 255));
	}

	static inline void Line(nc::Vector2f from, nc::Vector2f to, nc::Color col)
	{
#if NCINE_WITH_IMGUI && defined(WETPAPER_DEBUG)
		const float height = nc::theApplication().height();
		ImGui::GetForegroundDrawList()->AddLine(ImVec2(from.x, height - from.y), ImVec2(to.x, height - to.y), col.abgr());
#endif
	}

	static inline void Circle(nc::Vector2f center, float radius)
	{
		Circle(center, radius, nc::Color(255, 255, 255, 255));
	}

	static inline void Circle(nc::Vector2f center, float radius, nc::Color col)
	{
#if NCINE_WITH_IMGUI && defined(WETPAPER_DEBUG)
		const float height = nc::theApplication().height();
		ImGui::GetForegroundDrawList()->AddCircle(ImVec2(center.x, height - center.y), radius, col.abgr());
#endif
	}
};
