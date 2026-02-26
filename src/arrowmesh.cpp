/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "arrowmesh.h"
#include "renderservice.h"
#include "renderglobals.h"

// External Includes
#include <nap/core.h>
#include <nap/numeric.h>
#include <glm/detail/type_quat.hpp>
#include <meshutils.h>

// nap::ArrowMesh run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArrowMesh, "Arrow shape with uv, color and normal vertex attributes")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::ArrowMesh::mUsage,		nap::rtti::EPropertyMetaData::Default, "If the mesh is updated at runtime or static")
	RTTI_PROPERTY("CullMode",		&nap::ArrowMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default, "Which triangles are culled, front facing, back facing etc..")
	RTTI_PROPERTY("PolygonMode",	&nap::ArrowMesh::mPolygonMode,	nap::rtti::EPropertyMetaData::Default, "Polygon rasterization mode")
	RTTI_PROPERTY("Size",			&nap::ArrowMesh::mSize,			nap::rtti::EPropertyMetaData::Default, "Arrow dimensions")
	RTTI_PROPERTY("Color",			&nap::ArrowMesh::mColor,		nap::rtti::EPropertyMetaData::Default, "Arrow vertex color")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	// Arrow
	constexpr static float head_size = 0.3f;
	constexpr static float head_ext = 0.12f; // head extent
	constexpr static float tail_ext = 0.04f; // tail extent
	constexpr static float tail_end = 1.0f - head_size;

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


	ArrowMesh::ArrowMesh(Core& core) :
		mRenderService(core.getService<RenderService>()),
		mMeshInstance(std::make_unique<MeshInstance>(*core.getService<RenderService>()))
	{ }


	bool ArrowMesh::init(utility::ErrorState& errorState)
	{
		setup();
		return mMeshInstance->init(errorState);
	}


	void ArrowMesh::setup()
	{
		setup(math::Z_AXIS, *mMeshInstance);
	}


	void ArrowMesh::setup(const glm::vec3& direction, MeshInstance& mesh)
	{
		const uint count = sTriangles.size()*3;
		mesh.setNumVertices(count);
		mesh.setDrawMode(EDrawMode::Triangles);
		mesh.setCullMode(mCullMode);
		mesh.setUsage(mUsage);
		mesh.setPolygonMode(mPolygonMode);

		auto dir = glm::length(direction) > 1e-3f ? direction : math::Z_AXIS;
		dir = glm::normalize(dir);

		auto& pos_attr = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
		auto& norm_attr = mesh.getOrCreateAttribute<glm::vec3>(vertexid::normal);
		auto& uv_attr = mesh.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		auto& color_attr = mesh.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

		// Move verts by -0.5 along forward direction
		auto offset = dir * -0.5f;

		// Layout positions and normals
		std::vector<glm::vec3> positions;
		positions.reserve(count);

		std::vector<glm::vec3> normals;
		normals.reserve(count);

		for (const auto& tri : sTriangles)
		{
			// Compute flat shaded normals
			auto normal = glm::normalize(glm::cross(sVertices[tri[1]] - sVertices[tri[0]], sVertices[tri[2]] - sVertices[tri[0]]));
			for (int i = 0; i < tri.length(); i++)
			{
				positions.emplace_back(sVertices[tri[i]] + dir * offset);
				normals.emplace_back(normal);
			}
		}
		uv_attr.setData(positions);
		for (auto& p : positions)
			p *= mSize;

		pos_attr.setData(positions);
		norm_attr.setData(normals);
		color_attr.setData({count, mColor.toVec4()});

		// Index buffer is just positions in order
		auto& shape = mesh.createShape();
		utility::generateIndices(shape, count, false, 0);
	}
}
