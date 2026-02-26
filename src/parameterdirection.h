/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

// Local Includes
#include "mathutils.h"

// External Includes
#include <parameter.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * A numeric parameter that holds a normalized vec3 direction
	 */
	class NAPAPI ParameterDirection : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		/**
		 * Initialize
		 * @param errorState
		 * @return
		 */
		bool init(utility::ErrorState &errorState) override;

		/**
		 * Set the value of this enum from another parameter
		 * @param value The parameter to set the value from
		 */
		void setValue(const Parameter& value) override;

		/**
		 * Set the value of this parameter. Will raise the valueChanged signal if the value actually changes.
		 * @param value The value to set
		 */
		void setValue(const glm::vec3& value);

		glm::vec3				mValue;										        ///< Property: 'Value' the value of this parameter
		Signal<glm::vec3>		valueChanged;								        ///< Signal that's raised when the value changes
	};
}
