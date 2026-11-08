#include "CurveEditor.h"

#include <imguigizmoutils.h>
#include <imguiservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::gui::CurveEditor)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Curve", &nap::gui::CurveEditor::mCurve, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Position", &nap::gui::CurveEditor::mPosition, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size", &nap::gui::CurveEditor::mSize, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Margin", &nap::gui::CurveEditor::mMargin, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Indicator", &nap::gui::CurveEditor::mIndicator, nap::rtti::EPropertyMetaData::Default)
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
			ImGui::SetNextItemWidth(mSize.x * mIMGuiService->getScale());

			// The widget's screen rect is derived the same way ImGui::Curve derives it:
			// CalcItemWidth() returns the width set above and the height follows from the
			// aspect ratio. The last item rect cannot be used after the call, it points at
			// the last key point drawn inside the widget.
			const float scale = mIMGuiService->getScale();
			const ImVec2 min = ImGui::GetCursorScreenPos();
			const ImVec2 max(min.x + mSize.x * scale, min.y + mSize.y * scale);

			// ImGui::Curve insets the drawn area by the margin on all sides, so the
			// indicator is constrained to that inner box rather than the widget rect
			const float margin = mMargin * scale;
			const ImVec2 inner_min(min.x + margin, min.y + margin);
			const ImVec2 inner_max(max.x - margin, max.y - margin);

			// ImGui displays the text BEFORE the ### separator and hashes the widget id
			// from the text AFTER it: the id is mID, the displayed label is empty
			ImGui::Curve(("###" + mID).c_str(), *mCurve, mSize.x / mSize.y, mMargin * scale);

			// Draw the optional indicator as a vertical line over the curve widget, at the
			// x position corresponding to the parameter's value within its own min/max range
			if (mIndicator != nullptr)
			{
				const float range = mIndicator->mMaximum - mIndicator->mMinimum;
				if (range > 0.0f)
				{
					const float ratio = (mIndicator->mValue - mIndicator->mMinimum) / range;
					const float x = inner_min.x + ratio * (inner_max.x - inner_min.x);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(x, inner_min.y), ImVec2(x, inner_max.y), IM_COL32(255, 255, 0, 255), 1.0f);
				}
			}
		}

	}

}