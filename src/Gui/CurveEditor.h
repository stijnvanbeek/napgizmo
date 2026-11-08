#pragma once

#include <Gui/Gui.h>
#include <fcurve.h>
#include <nap/core.h>
#include <parameternumeric.h>

namespace nap
{

	namespace gui
	{

		class NAPAPI CurveEditor : public gui::Gui
		{
			RTTI_ENABLE(gui::Gui)

		public:
			CurveEditor(Core& core);

			ResourcePtr<math::FloatFCurve> mCurve; ///< Property: 'Curve' The curve being edited
			ResourcePtr<ParameterFloat> mIndicator = nullptr; ///< Property: 'Indicator' Optional parameter visualized as a vertical line at its value's position
			glm::vec2 mPosition = { 0.f, 0.f };
			glm::vec3 mSize = { 200.f, 100.f, 0.f };
			float mMargin = 5.f;

		private:
			void draw() override;
			IMGuiService* mIMGuiService = nullptr;
		};

	}

}