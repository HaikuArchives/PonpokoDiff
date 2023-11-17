/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef TEXTDIFFVIEW_H
#define TEXTDIFFVIEW_H


#include <View.h>

#include <vector>

#include "LineSeparatedText.h"
#include "NPDiff.h"

class BPath;


class DiffView : public BView {
public:
						DiffView(const char* name);
	virtual				~DiffView();

	virtual	void		MessageReceived(BMessage* message);

			void		Initialize();
			void		ExecuteDiff(BPath pathLeft, BPath pathRight);

private:
	enum PaneIndex {
		InvalidPane = -1,

		LEFT_PANE = 0,
		RIGHT_PANE,

		PaneMAX
	};

private:
			void		_PaneVScrolled(float y, DiffView::PaneIndex fromPaneIndex);

private:
	class DiffPaneView : public BView {
	public:
							DiffPaneView(const char* name);
		virtual				~DiffPaneView();

				void		SetDiffView(DiffView* fDiffView)
								{ this->fDiffView = fDiffView; }
				void		SetPaneIndex(DiffView::PaneIndex fPaneIndex)
								{ this->fPaneIndex = fPaneIndex; }
				void		DataChanged();

	public:
		virtual void		TargetedByScrollView(BScrollView* fScroller);
		virtual	void		Draw(BRect updateRect);
		virtual void		ScrollTo(BPoint point);
		virtual void		MouseDown(BPoint where);
		virtual	void		FrameResized(float width, float height);
		virtual void		SetFont(const BFont* font, uint32 properties = B_FONT_ALL);

	private:
				void		_RecalcLayout();
				void		_AdjustScrollBar();
				float		_GetDataHeight();
				float		_GetDataWidth();
				void		_DrawText(const BFont& font, const Substring& text, float baseLine);

	private:
		DiffView*			fDiffView;
		DiffView::PaneIndex	fPaneIndex;
		BScrollView*		fScroller;
		float				fDataHeight;
		float				fDataWidth;
		float				fTabUnit;
	};
	friend class DiffPaneView;

	struct LineInfo {
		int32 textIndex[PaneMAX];
		DiffOperation::Operator op;
	};
	typedef std::vector<LineInfo> LineInfoVector;

private:
		LineSeparatedText	fTextData[PaneMAX];
		LineInfoVector		fLineInfos;
		bool				fIsPanesVScrolling;
};

#endif // TEXTDIFFVIEW_H
