/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

// External Includes
#include <parameter.h>
#include <fcurve.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * A parameter that holds a float f-curve
	 */
	class NAPAPI ParameterFloatFCurve : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		/**
		 * Initialize
		 * @param errorState
		 * @return
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Set the value of this curve from another parameter
		 * @param value The parameter to set the value from
		 */
		void setValue(const Parameter& value) override;

		/**
		 * Set the value of this parameter. Will raise the valueChanged signal.
		 * @param value The value to set
		 */
		void setValue(const math::FloatFCurve& value);

		/**
		 * @return the float f-curve
		 */
		math::FloatFCurve& getValue() { return *mValue; }

		ResourcePtr<math::FloatFCurve> mValue;									///< Property: 'Value' the value of this parameter
		Signal<const math::FloatFCurve&> valueChanged;							///< Signal that's raised when the value changes
	};
}
