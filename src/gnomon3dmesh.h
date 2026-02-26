/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mesh.h"

namespace nap
{
	class RenderService;

	/**
	 * Predefined gnomon mesh consisting of three arrows representing the X, Y and Z axes.
	 * Includes uv, color and normal vertex attributes.
	 */
	class NAPAPI Gnomon3DMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		Gnomon3DMesh(Core& core);

		/**
		 * Sets up and initializes the gnomon as a mesh based on the provided parameters.
		 * @param errorState contains the error message if the mesh could not be created.
		 * @return if the mesh was successfully created and initialized.
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Creates and prepares the mesh but doesn't initialize it.
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

		float			mSize			= 1.0f;							///< Property: 'Size' size of the gnomon
		EMemoryUsage	mUsage			= EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		ECullMode		mCullMode		= ECullMode::Back;				///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.
		EPolygonMode	mPolygonMode	= EPolygonMode::Fill;			///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points)

	private:
		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
