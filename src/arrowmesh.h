/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mesh.h"

// External Includes
#include <color.h>

namespace nap
{
	class RenderService;

	/**
	 * Predefined box mesh with additional uv, color and normal vertex attributes.
	 * The UV coordinates are always 0-1. The box consists of 6 planes.
	 * The vertices of the individual planes are not shared.
	 */
	class NAPAPI ArrowMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		ArrowMesh(Core& core);

		/**
		 * Sets up and initializes the box as a mesh based on the provided parameters.
		 * @param errorState contains the error message if the mesh could not be created.
		 * @return if the mesh was successfully created and initialized.
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Creates and prepares the mesh but doesn't initialize it.
		 * Call this when you want to prepare a box without creating the GPU representation.
		 * You have to manually call init() on the mesh instance afterwards.
		 */
		void setup();

		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

		/**
		 * @return the box that wraps the mesh
		 */
		const glm::vec3& getDirection() const { return mDirection; }

		float			mSize			= 1.0f;							///< Property: 'Size' size of the arrow
		RGBAColorFloat	mColor			= { 1.0f, 1.0f, 1.0f, 1.0f };	///< Property: 'Color' color of the arrow
		EMemoryUsage	mUsage			= EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		ECullMode		mCullMode		= ECullMode::Back;				///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.
		EPolygonMode	mPolygonMode	= EPolygonMode::Fill;			///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points)

	protected:
		/**
		 * Constructs the mesh based on the given dimensions
		 * @param direction the direction to build the mesh from
		 * @param mesh the mesh that is constructed
		 */
		void setup(const glm::vec3& direction, MeshInstance& mesh);

	private:
		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;
		glm::vec3 mDirection = math::Z_AXIS;
	};
}
