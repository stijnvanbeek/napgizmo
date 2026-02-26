/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "parameterdirection.h"

RTTI_BEGIN_CLASS(nap::ParameterDirection)
	RTTI_PROPERTY("Value",	&nap::ParameterDirection::mValue,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	bool ParameterDirection::init(utility::ErrorState &errorState)
	{
		setValue(mValue);
		return true;
	}

	void ParameterDirection::setValue(const Parameter& value)
	{
		const auto* derived_type = rtti_cast<const ParameterDirection>(&value);
		assert(derived_type != nullptr);
		setValue(derived_type->mValue);
	}

	void ParameterDirection::setValue(const glm::vec3& value)
	{
		const auto old_value = mValue;
		if (value.x == 0.0f && value.y == 0.0f && value.z == 0.0f)
			mValue = math::Z_AXIS;
		else
			mValue = glm::normalize(value);

		if (old_value != mValue)
			valueChanged(mValue);
	}
}
