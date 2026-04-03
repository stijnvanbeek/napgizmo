/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imguigizmoutils.h"

// External Includes
#include <renderservice.h>
#include <algorithm>
#include <vector>
#include <renderglobals.h>
#include <triangleiterator.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ImGui
{
	bool drawTriangleMesh(const nap::MeshInstance& mesh, const glm::vec2& center, const float extent, const glm::mat4& transform, const glm::vec3& color)
	{
		// Simple 2D triangle struct
		struct Tri2D
		{
			glm::vec2 p[3];
			glm::vec3 diffuse;
			float z;
		};
		assert(mesh.getPolygonMode() == nap::EPolygonMode::Fill);
		auto& pos_attr = mesh.getAttribute<glm::vec3>(nap::vertexid::position);
		std::vector<Tri2D> triangles;
		triangles.reserve(pos_attr.getCount());

		nap::TriangleIterator it(mesh);
		while (!it.isDone())
		{
			const auto& tri = it.next();
			glm::vec3 v1 = transform * glm::vec4(pos_attr.getData()[tri[0]], 1.0f);
			glm::vec3 v2 = transform * glm::vec4(pos_attr.getData()[tri[1]], 1.0f);
			glm::vec3 v3 = transform * glm::vec4(pos_attr.getData()[tri[2]], 1.0f);
			auto normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

			// Backface culling (view direction -Z)
			if (glm::dot(normal, nap::math::Z_AXIS) < 0.0f)
				continue;

			auto& color_attr = mesh.getAttribute<glm::vec4>(nap::vertexid::getColorName(0));
			auto tri_color = color * glm::vec3(color_attr.getData()[tri[0]]);

			static const auto light_dir = glm::normalize(glm::vec3(1.0f, 1.0f, 2.0f));
			float diffuse = glm::dot(normal, light_dir);
			float avg_z = (v1.z + v2.z + v3.z) / 3.0f;

			// Projection: orthographic looking down -Z
			auto proj = [&](const glm::vec3& v) -> glm::vec2 {
				return center + glm::vec2(v.x, -v.y) * extent;
			};
			triangles.push_back({ { proj(v1), proj(v2), proj(v3) }, tri_color*diffuse, avg_z });
		}

		// Sort by depth (back to front)
		std::sort(triangles.begin(), triangles.end(), [](const Tri2D& a, const Tri2D& b) {
			return a.z < b.z;
		});

		auto* draw_list = GetWindowDrawList(); assert(draw_list != nullptr);
		for (const auto& tri : triangles)
		{
			draw_list->AddTriangleFilled(tri.p[0], tri.p[1], tri.p[2],
				ColorConvertFloat4ToU32(ImVec4(tri.diffuse.x, tri.diffuse.y, tri.diffuse.z, 1.0f)));
		}
		return true;
	}


	bool Direction(const nap::MeshInstance& arrow, const char* label, glm::vec3& direction)
	{
		if (GetCurrentContext()->CurrentWindow->SkipItems)
			return false;

		const float w = CalcItemWidth();
		const float h = w*0.5f;
		const ImRect bb(GetCursorScreenPos(), ImVec2(GetCursorScreenPos().x + w, GetCursorScreenPos().y + h));
		PushClipRect(bb.Min, bb.Max, true);
		ItemSize(bb, GetStyle().FramePadding.y);

		const auto id = GetID(label);
		if (!ItemAdd(bb, id))
		{
			PopClipRect();
			return false;
		}

		const glm::vec2 center = { bb.Min.x + w * 0.5f, bb.Min.y + h * 0.5f };
		const float extent = w * 0.4f;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		if (held)
		{
			const auto mouse_dx = ImVec2(GetIO().MouseDelta.x/extent, GetIO().MouseDelta.y/extent);
			auto dir = direction;
			dir = glm::length(dir) < 1e-6f ? nap::math::Z_AXIS : glm::normalize(dir);
			dir = glm::angleAxis(mouse_dx.x, nap::math::Y_AXIS) * dir;
			dir = glm::angleAxis(mouse_dx.y, nap::math::X_AXIS) * dir;
			dir = glm::normalize(dir);
			direction = dir;
			pressed = true;
		}

		// Rendering
		auto* draw_list = GetWindowDrawList(); assert(draw_list != nullptr);
		draw_list->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), GetStyle().FrameRounding);

		// Draw arrow
		if (glm::length(direction) > 1e-6f)
		{
			auto dir = glm::normalize(direction);
			auto forward = dir;
			auto up = std::abs(forward.y) < 1.0f - 1e-6f ? nap::math::Y_AXIS : nap::math::X_AXIS;
			auto right = glm::normalize(glm::cross(up, forward));
			up = glm::cross(forward, right);

			const glm::mat4 transform(
				glm::vec4(right, 0.0f),
				glm::vec4(up, 0.0f),
				glm::vec4(forward, 0.0f),
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
			);

			// Light direction in world space (towards the light)
			drawTriangleMesh(arrow, center, extent, transform, dir*0.5f+0.5f);
		}

		// Value text
		const auto value_pos = ImVec2(bb.Min.x + GetStyle().ItemInnerSpacing.x, bb.Min.y + GetStyle().FramePadding.y);
		const auto value_str = nap::utility::stringFormat("%.03f, %.03f, %.03f", direction.x, direction.y, direction.z);
		RenderText(value_pos, value_str.c_str());
		PopClipRect();

		// Label
		const auto label_pos = ImVec2(bb.Max.x + GetStyle().ItemInnerSpacing.x, bb.Min.y + GetStyle().FramePadding.y);
		RenderText(label_pos, label);

		return pressed;
	}


	bool Rotation(const nap::MeshInstance& gnomon, const char* label, glm::quat& rotation)
	{
		if (GetCurrentContext()->CurrentWindow->SkipItems)
			return false;

		const float w = CalcItemWidth();
		const float h = w * 0.5f;
		const ImRect bb(GetCursorScreenPos(), ImVec2(GetCursorScreenPos().x + w, GetCursorScreenPos().y + h));
		PushClipRect(bb.Min, bb.Max, false);
		ItemSize(bb, GetStyle().FramePadding.y);

		const auto id = GetID(label);
		if (!ItemAdd(bb, id))
		{
			PopClipRect();
			return false;
		}

		const glm::vec2 center = { bb.Min.x + w * 0.5f, bb.Min.y + h * 0.5f };
		const float extent = w * 0.4f;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		if (held)
		{
			const auto mouse_dx = ImVec2(GetIO().MouseDelta.x/extent, GetIO().MouseDelta.y/extent);
			rotation = glm::angleAxis(mouse_dx.x, nap::math::Y_AXIS) * rotation;
			rotation = glm::angleAxis(mouse_dx.y, nap::math::X_AXIS) * rotation;
			rotation = glm::normalize(rotation);
			pressed = true;
		}

		// Rendering
		auto* draw_list = GetWindowDrawList(); assert(draw_list != nullptr);
		draw_list->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), GetStyle().FrameRounding);

		// Gnomon has its own colors, so we pass white as base color to DrawMesh
		drawTriangleMesh(gnomon, center, extent, glm::mat4_cast(rotation), glm::vec3(1.0f));

		// Value text
		const auto value_pos = ImVec2(bb.Min.x + GetStyle().ItemInnerSpacing.x, bb.Min.y + GetStyle().FramePadding.y);
		const auto value_str = nap::utility::stringFormat("%.03f, %.03f, %.03f, %.03f", rotation.w, rotation.x, rotation.y, rotation.z);
		RenderText(value_pos, value_str.c_str());
		PopClipRect();

		// Label
		const auto label_pos = ImVec2(bb.Max.x + GetStyle().ItemInnerSpacing.x, bb.Min.y + GetStyle().FramePadding.y);
		RenderText(label_pos, label);

		return pressed;
	}


	bool Curve(const char* label, nap::math::FloatFCurve& curve)
	{
	    if (GetCurrentContext()->CurrentWindow->SkipItems)
	    	return false;

		// Draw frame
		float w = CalcItemWidth();
		float h = w * 0.5f;
		ImRect bb(GetCursorScreenPos(), ImVec2(GetCursorScreenPos().x + w, GetCursorScreenPos().y + h));
		const ImRect bb_outer = bb;
		PushClipRect(bb.Min, bb.Max, false);
		ItemSize(bb, GetStyle().FramePadding.y);

		auto id = GetID(label);
		if (!ItemAdd(bb, id))
		{
			PopClipRect();
			return false;
		}

		// Draw canvas rectangle
		auto* draw_list = GetWindowDrawList();
		draw_list->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), GetStyle().FrameRounding);

		const float margin = w*0.05f;
		w = w - margin*2.0f;
		h = h - margin*2.0f;
		ImVec2 tl(bb.GetTL().x + margin, bb.GetTL().y + margin);
		bb = ImRect(tl, ImVec2(tl.x + w, tl.y + h));
		id = GetID(nap::utility::stringFormat("%s_box", label).c_str());
		if (!ItemAdd(bb, id))
		{
			PopClipRect();
			return false;
		}

		draw_list->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_WindowBg), GetStyle().FrameRounding, 0, 2);

		// Bail if there are less than two curve points
		// TODO: Account for this situation in the widget
		if (curve.mPoints.size() < 2)
		{
			TextDisabled("Curve needs at least two points");
			return false;
		}

		float max_time = curve.mPoints.back().mPos.mTime;
		if (max_time <= 0.0f)
			max_time = 1.0f;

		bool value_changed = false;

	    // Double-click to add a point
	    if (IsItemHovered() && IsMouseDoubleClicked(0))
	    {
	    	float new_t = ((GetIO().MousePos.x - bb.Min.x) / w) * max_time;
	    	float new_v = 1.0f - (GetIO().MousePos.y - bb.Min.y) / h;
	    	curve.mPoints.push_back({{new_t, new_v}, {-0.1f, 0.0f}, {0.1f, 0.0f}});
	    	curve.invalidate();
	    	value_changed = true;
	    }

	    auto translate = [&](float t, float v) {
	        return ImVec2(bb.Min.x + (t / max_time) * w, bb.Max.y - v * h);
	    };

		// Sort points
		std::vector<nap::math::FloatFCurvePoint*> sorted;
		for (auto& p : curve.mPoints)
			sorted.emplace_back(&p);

		std::sort(curve.mPoints.begin(), curve.mPoints.end(), [](const nap::math::FloatFCurvePoint& lhs, const nap::math::FloatFCurvePoint& rhs) {
			return lhs.mPos.mTime < rhs.mPos.mTime;
		});

	    constexpr int segment_sample_rate = 8;
		const int segment_count = sorted.size()*segment_sample_rate;
		std::vector<glm::vec2> bezier;
		bezier.reserve(segment_count);

		for (size_t i = 0; i < sorted.size()-1; ++i)
		{
			const auto& p0 = sorted[i]->mPos;
			const auto& p1 = sorted[i+1]->mPos;
			const float dx = p1.mTime-p0.mTime;
			const float step = dx/static_cast<float>(segment_count);
			for (int j = 0; j <= segment_count; ++j)
			{
				const float t = p0.mTime + j*step;
				bezier.emplace_back(t, curve.evaluate(t));
			}
		}

		// Bezier lines
		for (int i = 0; i < bezier.size()-1; ++i)
		{
			const auto& p0 = bezier[i];
			const auto& p1 = bezier[i+1];
			draw_list->AddLine(translate(p0.x, p0.y), translate(p1.x, p1.y), GetColorU32(ImGuiCol_PlotLines), 2.0f);
		}

		// Key points
		int point_to_delete = -1;
	    for (int i = 0; i < sorted.size(); ++i)
	    {
	    	auto& p0 = sorted[i]->mPos;
	    	const auto& p1 = sorted[i+1]->mPos;

	    	constexpr float hov_rad = 10.0f;
	        ImVec2 hov_pos = translate(p0.mTime, p0.mValue);
	        ImRect hov_bb(ImVec2(hov_pos.x - hov_rad, hov_pos.y - hov_rad), ImVec2(hov_pos.x + hov_rad, hov_pos.y + hov_rad));

	    	const auto hov_id = GetID(nap::utility::stringFormat("point%d", i).c_str());
	    	if (!ItemAdd(hov_bb, hov_id))
	    		return false;

	    	bool hovered, held;
	        ButtonBehavior(hov_bb, hov_id, &hovered, &held);

	    	// Mark point for deletion
	    	// Ensure the first and last points cannot be deleted
	    	if (hovered && i!=0 && i!=sorted.size()-1 && GetIO().MouseClicked[1])
	    		point_to_delete = i;

	    	// Drag point
	        if (held)
	        {
	        	// Drag point
	        	const auto mouse_dx = GetIO().MouseDelta;

	            // Constrain time within neighbors
	            float min_t = (i == 0) ? 0.0f : sorted[i-1]->mPos.mTime + 1e-3f;
	            float max_t = (i == curve.mPoints.size()-1) ? max_time : p1.mTime - 1e-3f;

	        	// Snap first and last point to 0 and 1
	        	min_t = (i == curve.mPoints.size()-1) ? 1.0f : min_t;
	        	max_t = (i == 0) ? 0.0f : max_t;

	        	// Update time
	            float new_t = p0.mTime + (mouse_dx.x/w) * max_time;
				p0.mTime = glm::clamp(new_t, min_t, max_t);

	            // Update value
	            float new_v = p0.mValue - mouse_dx.y/h;
	            p0.mValue = glm::clamp(new_v, 0.0f, 1.0f);
	            value_changed = true;
	        }
	        ImU32 col = GetColorU32(hovered||held ? ImGuiCol_ButtonHovered : ImGuiCol_SliderGrab);
	        draw_list->AddCircleFilled(hov_pos, hov_rad*0.75f, col);
	    }
		PopClipRect();

		// Delete the point after rendering
		if (point_to_delete != -1)
		{
			auto it = std::find_if(curve.mPoints.begin(), curve.mPoints.end(), [p=sorted[point_to_delete]](auto& other) {
				return p == &other;
			});
			assert(it != curve.mPoints.end());
			curve.mPoints.erase(it);
			curve.invalidate();
			value_changed = true;
		}

		// Label
		const auto label_pos = ImVec2(bb_outer.Max.x + GetStyle().ItemInnerSpacing.x, bb_outer.Min.y + GetStyle().FramePadding.y);
		RenderText(label_pos, label);

	    return value_changed;
	}
}
