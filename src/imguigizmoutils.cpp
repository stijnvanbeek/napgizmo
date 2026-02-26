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

#include <imgui_internal.h>
#include <imgui.h>

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

		ImDrawList* draw_list = GetWindowDrawList(); assert(draw_list != nullptr);
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
			return false;

		const glm::vec2 center = { bb.Min.x + w * 0.5f, bb.Min.y + h * 0.5f };
		const float extent = w * 0.4f;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		if (held)
		{
			const auto mouse_delta = ImVec2(GetIO().MouseDelta.x/extent, GetIO().MouseDelta.y/extent);
			auto dir = direction;
			dir = glm::length(dir) < 1e-6f ? nap::math::Z_AXIS : glm::normalize(dir);
			dir = glm::angleAxis(mouse_delta.x, nap::math::Y_AXIS) * dir;
			dir = glm::angleAxis(mouse_delta.y, nap::math::X_AXIS) * dir;
			dir = glm::normalize(dir);
			direction = dir;
			pressed = true;
		}

		// Rendering
		ImDrawList* draw_list = GetWindowDrawList(); assert(draw_list != nullptr);
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
		const ImRect bb(GetCursorScreenPos(), ImVec2(GetCursorScreenPos().x + w, GetCursorScreenPos().y +h));
		PushClipRect(bb.Min, bb.Max, false);
		ItemSize(bb, GetStyle().FramePadding.y);

		const auto id = GetID(label);
		if (!ItemAdd(bb, id))
			return false;

		const glm::vec2 center = { bb.Min.x + w * 0.5f, bb.Min.y + h * 0.5f };
		const float extent = w * 0.4f;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		if (held)
		{
			const auto mouse_delta = ImVec2(GetIO().MouseDelta.x/extent, GetIO().MouseDelta.y/extent);
			rotation = glm::angleAxis(mouse_delta.x, nap::math::Y_AXIS) * rotation;
			rotation = glm::angleAxis(mouse_delta.y, nap::math::X_AXIS) * rotation;
			rotation = glm::normalize(rotation);
			pressed = true;
		}

		// Rendering
		ImDrawList* draw_list = GetWindowDrawList(); assert(draw_list != nullptr);
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
}
