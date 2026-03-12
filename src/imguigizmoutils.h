/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

// External Includes
#include <mesh.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>
#include <fcurve.h>

/**
 * This file contains NAP overrides for popular IMGui functions.
 * These utility functions allow you to use common NAP objects in conjunction with IMGui
 * All the functions in this file follow the IMGui style and naming conventions
 */
namespace ImGui
{
	/**
	 * Widget for editing a normalized 3D direction vector.
	 * Allows dragging a 3D arrow to adjust the parameter.
	 * @param label the label of the widget
	 * @param direction the direction normalized vector to edit, xyz only where w equals zero
	 * @return true if the direction was changed
	 */
	bool NAPAPI Direction(const nap::MeshInstance& arrow, const char* label, glm::vec3& direction);

	/**
	 * Widget for editing an orientation quaternion.
	 * Allows dragging a 3D gnomon to adjust the parameter.
	 * @param gnomon the gnomon mesh to render
	 * @param label the label of the widget
	 * @param rotation the orientation quaternion to edit
	 * @return true if the orientation was changed
	 */
	bool NAPAPI Rotation(const nap::MeshInstance& gnomon, const char* label, glm::quat& rotation);

	/**
	 * Widget for editing float curves.
	 * @return
	 */
	bool NAPAPI Curve(const char* label, nap::math::FloatFCurve& curve);
}
