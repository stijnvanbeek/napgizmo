/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rendercomponent.h>
#include <componentptr.h>
#include <materialinstance.h>
#include <renderablemesh.h>
#include <gnomon3dmesh.h>

namespace nap
{
	// Forward Declares
	class RenderGizmoComponentInstance;
	class TransformComponentInstance;

	/**
	 * RenderGizmoComponent
	 */
	class NAPAPI RenderGizmoComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderGizmoComponent, RenderGizmoComponentInstance)
	public:
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * RenderGizmoComponentInstance
	 */
	class NAPAPI RenderGizmoComponentInstance : public  RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderGizmoComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderGizmoComponentInstance based on the RenderGizmoComponent resource.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderGizmoComponentInstance initialized successfully
		 */
		bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Draws the effect full screen to the currently active render target
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix ignored
		 * @param projectionMatrix ignored
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		RenderService&					mRenderService;						///< Render service
		RenderGizmoComponent*			mResource = nullptr;
		TransformComponentInstance*		mTransform;				///< Cached pointer to transform

		Material*						mMaterial = nullptr;				///< The Material
		MaterialInstance				mMaterialInstance;					///< The MaterialInstance as created from the resource
		MaterialInstanceResource		mMaterialInstanceResource;			///< The MaterialInstance resource
		RenderableMesh					mRenderableMesh;					///< The currently active renderable mesh, either set during init() or set by setMesh.

		UniformMat4Instance*			mModelMatUniform = nullptr;			///< Pointer to the model matrix uniform
		UniformMat4Instance*			mViewMatUniform = nullptr;			///< Pointer to the view matrix uniform
		UniformMat4Instance*			mProjectMatUniform = nullptr;		///< Pointer to the projection matrix uniform
		UniformMat4Instance*			mNormalMatrixUniform = nullptr;		///< Pointer to the normal matrix uniform
		UniformVec3Instance*			mCameraWorldPosUniform = nullptr;	///< Pointer to the camera world position uniform

		std::unique_ptr<Gnomon3DMesh>	mGnomonMesh;						///< Gnomon mesh
	};
}
