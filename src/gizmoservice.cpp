/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gizmoservice.h"

// External Includes
#include <nap/core.h>
#include <renderservice.h>
#include <parameterguiservice.h>
#include <parameterdirection.h>
#include <parameterquat.h>
#include <imguigizmoutils.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GizmoService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	GizmoService::GizmoService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	bool GizmoService::init(utility::ErrorState& errorState)
	{
		mArrowMesh = std::make_unique<ArrowMesh>(getCore());
		if (!mArrowMesh->init(errorState))
			return false;

		mGnomonMesh = std::make_unique<Gnomon3DMesh>(getCore());
		mGnomonMesh->mSize = 0.5f;
		if (!mGnomonMesh->init(errorState))
			return false;

		// Register parameter editors
		auto* service = getCore().getService<ParameterGUIService>();
		assert(service != nullptr);

		service->registerParameterEditor(RTTI_OF(ParameterDirection), [arrow = mArrowMesh.get()](Parameter& parameter)
		{
			auto* param = static_cast<ParameterDirection*>(&parameter);
			auto value = param->mValue;
			if (ImGui::Direction(arrow->getMeshInstance(), param->mName.c_str(), value))
				param->setValue(value);
		});
		service->registerParameterEditor(RTTI_OF(ParameterQuat), [gnomon = mGnomonMesh.get()](Parameter& parameter)
		{
			auto* param = static_cast<ParameterQuat*>(&parameter);
			auto value = param->mValue;
			if (ImGui::Rotation(gnomon->getMeshInstance(), param->mName.c_str(), value))
				param->setValue(value);
		});
		return true;
	}


	void GizmoService::preShutdown()
	{
		mArrowMesh.reset();
		mGnomonMesh.reset();
	}


	void GizmoService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(RenderService));
		dependencies.emplace_back(RTTI_OF(ParameterGUIService));
	}
}