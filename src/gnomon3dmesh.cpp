/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gnomon3dmesh.h"
#include "renderservice.h"
#include "renderglobals.h"

// External Includes
#include <nap/core.h>
#include <nap/numeric.h>
#include <glm/gtx/quaternion.hpp>
#include <meshutils.h>

// nap::Gnomon3DMesh run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Gnomon3DMesh, "Gnomon shape consisting of three arrows (X, Y, Z)")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::Gnomon3DMesh::mUsage,			nap::rtti::EPropertyMetaData::Default, "If the mesh is updated at runtime or static")
	RTTI_PROPERTY("CullMode",		&nap::Gnomon3DMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default, "Which triangles are culled, front facing, back facing etc..")
	RTTI_PROPERTY("PolygonMode",	&nap::Gnomon3DMesh::mPolygonMode,	nap::rtti::EPropertyMetaData::Default, "Polygon rasterization mode")
	RTTI_PROPERTY("Size",			&nap::Gnomon3DMesh::mSize,			nap::rtti::EPropertyMetaData::Default, "Gnomon dimensions")
RTTI_END_CLASS


namespace nap
{
	// Arrow constants
	constexpr static float head_size = 0.15f;
	constexpr static float head_ext = 0.06f; // head extent
	constexpr static float tail_ext = 0.02f; // tail extent
	constexpr static float tail_end = 1.0f - head_size;

	// Arrow vertices along Z axis
	const static std::vector<glm::vec3> sVertices =
	{
		{ -tail_ext, -tail_ext, 0 },
		{ tail_ext, -tail_ext, 0 },
		{ tail_ext, tail_ext, 0 },
		{ -tail_ext, tail_ext, 0 },
		{ -tail_ext, -tail_ext, tail_end },
		{ tail_ext, -tail_ext, tail_end },
		{ tail_ext, tail_ext, tail_end },
		{ -tail_ext, tail_ext, tail_end },
		{ -head_ext, -head_ext, tail_end },
		{ head_ext, -head_ext, tail_end },
		{ head_ext, head_ext, tail_end },
		{ -head_ext, head_ext, tail_end },
		{ 0.0f, 0.0f, 1.0f }
	};

	const static std::vector<glm::uvec3> sTriangles =
	{
		{ 0, 1, 5 },	{ 0, 5, 4 },	// Bottom
		{ 1, 2, 6 },	{ 1, 6, 5 },	// Right
		{ 2, 3, 7 },	{ 2, 7, 6 },	// Top
		{ 3, 0, 4 },	{ 3, 4, 7 },	// Left
		{ 3, 2, 1 },	{ 3, 1, 0 },	// Base
		{ 8, 9, 12 },	{ 9, 10, 12 },	// Head
		{ 10, 11, 12 }, { 11, 8, 12 },
		{ 11, 10, 9 },	{ 11, 9, 8 }	// Head base
	};


	Gnomon3DMesh::Gnomon3DMesh(Core& core) :
		mRenderService(core.getService<RenderService>()),
		mMeshInstance(std::make_unique<MeshInstance>(*core.getService<RenderService>()))
	{ }


	bool Gnomon3DMesh::init(utility::ErrorState& errorState)
	{
		setup();
		return mMeshInstance->init(errorState);
	}


	void Gnomon3DMesh::setup()
	{
		const uint arrow_count = sTriangles.size() * 3;
		const uint vert_count = arrow_count * 3;

		mMeshInstance->setNumVertices(vert_count);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setCullMode(mCullMode);
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setPolygonMode(mPolygonMode);

		auto& pos_attr = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		auto& norm_attr = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::normal);
		auto& uv_attr = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		auto& color_attr = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

		std::vector<glm::vec3> positions;	positions.reserve(vert_count);
		std::vector<glm::vec3> normals;		normals.reserve(vert_count);
		std::vector<glm::vec3> uvs;			uvs.reserve(vert_count);
		std::vector<glm::vec4> colors;		colors.reserve(vert_count);

		auto add_arrow = [&](const glm::vec3& direction)
		{
			auto rotation = glm::rotation(math::Z_AXIS, direction);
			for (const auto& tri : sTriangles)
			{
				// Compute flat shaded normal
				auto v0 = sVertices[tri[0]];
				auto v1 = sVertices[tri[1]];
				auto v2 = sVertices[tri[2]];
				auto normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

				for (int i = 0; i < tri.length(); i++)
				{
					auto offset = glm::vec3(0.0f, 0.0f, tail_ext);
					auto pos = rotation * (sVertices[tri[i]] + offset);
					uvs.emplace_back(pos);
					positions.emplace_back(pos * mSize);
					normals.emplace_back(rotation * normal);
					colors.emplace_back(glm::vec4(direction, 1.0f));
				}
			}
		};
		add_arrow(math::X_AXIS);
		add_arrow(math::Y_AXIS);
		add_arrow(math::Z_AXIS);

		pos_attr.setData(positions);
		norm_attr.setData(normals);
		uv_attr.setData(uvs);
		color_attr.setData(colors);

		// Index buffer is just positions in order
		auto& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, vert_count, false, 0);
	}
}
