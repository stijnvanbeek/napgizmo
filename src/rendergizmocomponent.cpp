/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendergizmocomponent.h"
#include "gizmoshader.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <transformcomponent.h>

// nap::RenderGizmoComponent run time class definition
RTTI_BEGIN_CLASS(nap::RenderGizmoComponent)
RTTI_END_CLASS

// nap::RenderGizmoComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderGizmoComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderGizmoComponent
	//////////////////////////////////////////////////////////////////////////

	void RenderGizmoComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderGizmoComponentInstance
	//////////////////////////////////////////////////////////////////////////

	RenderGizmoComponentInstance::RenderGizmoComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(*entity.getCore()->getService<RenderService>()),
		mGnomonMesh(std::make_unique<Gnomon3DMesh>(*entity.getCore()))
	{ }


	bool RenderGizmoComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		mResource = getComponent<RenderGizmoComponent>();

		// Ensure there is a transform component
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Create gnomon mesh
		if (!mGnomonMesh->init(errorState))
			return false;

		// Initialize material based on resource
		mMaterial = mRenderService.getOrCreateMaterial<GizmoShader>(errorState); assert(mMaterial != nullptr);
		if (!errorState.check(mMaterial != nullptr, "%s: unable to get or create homography material", mResource->mID.c_str()))
			return false;

		mMaterialInstanceResource.mMaterial = mMaterial;
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::ReadWrite;
		if (!mMaterialInstance.init(mRenderService, mMaterialInstanceResource, errorState))
			return false;

		mRenderableMesh = mRenderService.createRenderableMesh(*mGnomonMesh, mMaterialInstance, errorState);
		if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
			return false;

		// Since the material can't be changed at run-time, cache the matrices to set on draw
		// If the struct is found, we expect the matrices with those names to be there
		auto* mvp = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (mvp != nullptr)
		{
			mModelMatUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);
			mViewMatUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
			mProjectMatUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
			mNormalMatrixUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::normalMatrix);
			mCameraWorldPosUniform = mvp->getOrCreateUniform<UniformVec3Instance>(uniform::cameraPosition);
		}

		return true;
	}


	void RenderGizmoComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		if (!isVisible())
			return;

		// Set mvp matrices if present in material
		if (mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		if (mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if (mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransform->getGlobalTransform());

		if (mNormalMatrixUniform != nullptr)
			mNormalMatrixUniform->setValue(glm::transpose(glm::inverse(mTransform->getGlobalTransform())));

		if (mCameraWorldPosUniform != nullptr)
			mCameraWorldPosUniform->setValue(math::extractPosition(glm::inverse(viewMatrix)));

		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService.getOrCreatePipeline(renderTarget, *mGnomonMesh, mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		const auto& descriptor_set = mMaterialInstance.update();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const auto& buffers = mRenderableMesh.getVertexBuffers();
		const auto& offsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
		vkCmdDraw(commandBuffer, mGnomonMesh->getMeshInstance().getNumVertices(), 1, 0, 0);
	}
}
