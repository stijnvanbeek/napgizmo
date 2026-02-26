/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>

// Local includes
#include <arrowmesh.h>
#include <gnomon3dmesh.h>

namespace nap
{
	class ParameterGUIService;

	/**
	 * GizmoService
	 */
	class NAPAPI GizmoService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 * Default constructor. This service has no settings associated with it.
		 */
		GizmoService(ServiceConfiguration* configuration);

		// Destructor
		~GizmoService() override {}

	protected:
		/**
		 * Prints all the available network adapters to console.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Use this call to register service dependencies
		 * A service that depends on another service is initialized after all it's associated dependencies
		 * This will ensure correct order of initialization, update calls and shutdown of all services
		 * @param dependencies rtti information of the services this service depends on
		 */
		void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Invoked when exiting the main loop, after app shutdown is called
		 * This is called before shutdown() and before the resources are destroyed.
		 * Use this function if your service needs to reset its state before resources
		 * are destroyed
		 * When service B depends on A, Service B is shutdown before A
		 */
		void preShutdown() override;

	private:
		std::unique_ptr<ArrowMesh> mArrowMesh;
		std::unique_ptr<Gnomon3DMesh> mGnomonMesh;
	};
}
