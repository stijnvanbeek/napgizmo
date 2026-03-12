/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "parameterfloatfcurve.h"

RTTI_BEGIN_CLASS(nap::ParameterFloatFCurve)
	RTTI_PROPERTY("Value",	&nap::ParameterFloatFCurve::mValue,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	bool ParameterFloatFCurve::init(utility::ErrorState &errorState)
	{
		setValue(*mValue);
		return true;
	}

	void ParameterFloatFCurve::setValue(const Parameter& value)
	{
		const auto* derived_type = rtti_cast<const ParameterFloatFCurve>(&value);
		assert(derived_type != nullptr);
		setValue(*derived_type->mValue);
	}

	void ParameterFloatFCurve::setValue(const math::FloatFCurve& value)
	{
		mValue->mPoints = value.mPoints;
		mValue->invalidate();
		valueChanged(*mValue);
	}
}
