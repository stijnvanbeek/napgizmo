#pragma once

#include <Gui/Gui.h>

namespace nap
{

	namespace gui
	{

		class NAPAPI CurveEditor : public gui::Gui
		{
			RTTI_ENABLE(gui::Gui)

		public:
			CurveEditor(Core& core);

			ResourcePtr<math::FloatFCurve> mCurve; ///< Property:
			glm::vec2 mPosition = { 0.f, 0.f };
			float mWidth = 200.f;
			float mAspectRatio = 2.f;
			float mMargin = 5.f;

		private:
			void draw() override;
			IMGuiService* mIMGuiService = nullptr;
		};

	}

}