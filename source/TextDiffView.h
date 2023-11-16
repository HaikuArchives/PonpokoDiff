/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef TEXTDIFFVIEW_H__INCLUDED
#define TEXTDIFFVIEW_H__INCLUDED


#include <View.h>

#include <vector>

#include "LineSeparatedText.h"
#include "NPDiff.h"

class BPath;


class TextDiffView : public BView {
public:
						TextDiffView(const char* name);
	virtual				~TextDiffView();

	virtual	void		MessageReceived(BMessage* message);

			void		Initialize();
			void		ExecuteDiff(BPath pathLeft, BPath pathRight);

private:
	enum PaneIndex {
		InvalidPane = -1,

		LeftPane = 0,
		RightPane,

		PaneMAX
	};

private:
			void		paneVScrolled(float y, TextDiffView::PaneIndex fromPaneIndex);
			void		makeFocusToPane(TextDiffView::PaneIndex paneIndex);

private:
	class DiffPaneView : public BView {
	public:
							DiffPaneView(const char* name);
		virtual				~DiffPaneView();

				void		SetTextDiffView(TextDiffView* textDiffView)
								{ this->textDiffView = textDiffView; }
				void		SetPaneIndex(TextDiffView::PaneIndex paneIndex)
								{ this->paneIndex = paneIndex; }
				void		DataChanged();

	public:
		virtual void		TargetedByScrollView(BScrollView* scroller);
		virtual	void		Draw(BRect updateRect);
		virtual void		ScrollTo(BPoint point);
		virtual void		MouseDown(BPoint where);
		virtual	void		FrameResized(float width, float height);
		virtual void		SetFont(const BFont* font, uint32 properties = B_FONT_ALL);

	private:
				void		recalcLayout();
				void		adjustScrollBar();
				float		getDataHeight();
				void		drawText(const BFont& font, const Substring& text, float baseLine);

	private:
		TextDiffView*		textDiffView;
		TextDiffView::PaneIndex	paneIndex;
		BScrollView*		scroller;
		float				dataHeight;
		float				tabUnit;
		float				maxLineLength;
	};
	friend class DiffPaneView;

	struct LineInfo {
		int32 textIndex[PaneMAX];
		DiffOperation::Operator op;
	};
	typedef std::vector<LineInfo> LineInfoVector;

private:
		LineSeparatedText	textData[PaneMAX];
		LineInfoVector		lineInfos;
		bool				isPanesVScrolling;
};

#endif // TEXTDIFFVIEW_H__INCLUDED
