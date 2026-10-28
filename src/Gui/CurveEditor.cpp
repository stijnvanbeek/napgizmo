#include "CurveEditor.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::gui::CurveEditor)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Curve", &nap::gui::CurveEditor::mCurve, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Position", &nap::gui::CurveEditor::mPosition, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Width", &nap::gui::CurveEditor::mWidth, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AspectRatio", &nap::gui::CurveEditor::mAspectRatio, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Margin", &nap::gui::CurveEditor::mMargin, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{

	namespace gui
	{

		CurveEditor::CurveEditor(Core &core)
		{
			mIMGuiService = core.getService<IMGuiService>();
		}


		void CurveEditor::draw()
		{
			ImGui::SetCursorPos(ImVec2(mPosition.x * mIMGuiService->getScale(), mPosition.y * mIMGuiService->getScale()));
			ImGui::SetNextItemWidth(mWidth * mIMGuiService->getScale());
			ImGui::Curve(("###" + mID).c_str(), *mCurve, mAspectRatio, mMargin * mIMGuiService->getScale());
		}

	}

}