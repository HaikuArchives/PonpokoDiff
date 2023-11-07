/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Mark Hellegers
 *		Humdinger
 *
 */

#include "TextDiffView.h"

#include <math.h>

#include "Exception.h"
#include <ControlLook.h>
#include <ScrollBar.h>
#include <ScrollView.h>


static const char FONT_SAMPLE[] = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int FONT_SAMPLE_LENGTH = sizeof(FONT_SAMPLE) - 1;

static const rgb_color colorEmpty = {192, 192, 192, 255};
static const rgb_color colorInserted = {224, 255, 128, 255};
static const rgb_color colorDeleted = {255, 224, 128, 255};
static const rgb_color colorModified = {255, 255, 128, 255};

static const int tabChars = 4;

#define PANE_SPLITTER_WIDTH 6
#define HORIZONTAL_SCROLL_MAX 4000
#define HORIZONTAL_SCROLL_MINSTEPS 8


TextDiffView::TextDiffView(BRect frame, const char* name, uint32 resizingMode)
	:
	BView(frame, name, resizingMode, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	isPanesVScrolling = false;
}


TextDiffView::~TextDiffView()
{
}


void
TextDiffView::Initialize()
{
	BRect bounds = Bounds();
	float leftWidth = floor((bounds.Width() + 1 - be_control_look->GetScrollBarWidth(B_HORIZONTAL)
		- PANE_SPLITTER_WIDTH) / 2);

	BRect leftFrame = BRect(bounds.left, bounds.top, bounds.left + leftWidth - 1,
		bounds.bottom - be_control_look->GetScrollBarWidth(B_HORIZONTAL));
	DiffPaneView* leftPaneView = new DiffPaneView(leftFrame, "LeftPane", B_FOLLOW_ALL_SIDES);
	leftPaneView->SetTextDiffView(this);
	leftPaneView->SetPaneIndex(LeftPane);

	BScrollView* leftView = new BScrollView("LeftPaneScroller", leftPaneView, B_FOLLOW_NONE,
		B_FRAME_EVENTS, true, false, B_NO_BORDER);
	AddChild(leftView);

	BRect rightFrame = BRect(bounds.left + leftWidth + PANE_SPLITTER_WIDTH, bounds.top,
		bounds.right - be_control_look->GetScrollBarWidth(B_VERTICAL),
		bounds.bottom - be_control_look->GetScrollBarWidth(B_HORIZONTAL));
	DiffPaneView* rightPaneView = new DiffPaneView(rightFrame, "RightPane", B_FOLLOW_ALL_SIDES);
	rightPaneView->SetTextDiffView(this);
	rightPaneView->SetPaneIndex(RightPane);

	BScrollView* rightView = new BScrollView("RightPaneScroller", rightPaneView, B_FOLLOW_NONE,
		B_FRAME_EVENTS, true, true, B_NO_BORDER);
	AddChild(rightView);

	// Don't let the app server erase the view.
	// We do all drawing ourselves, so it is not necessary and only causes flickering
	SetViewColor(B_TRANSPARENT_COLOR);
	// The splitter in the middle will be drawn with the low color
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);

	recalcLayout();

	leftPaneView->DataChanged();
	rightPaneView->DataChanged();
}


void
TextDiffView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
	recalcLayout();
}


void
TextDiffView::recalcLayout()
{
	BRect bounds = Bounds();
	float leftWidth = floor((bounds.Width() + 1 - be_control_look->GetScrollBarWidth(B_HORIZONTAL)
		- PANE_SPLITTER_WIDTH) / 2);
	float rightWidth = (bounds.Width() + 1 - PANE_SPLITTER_WIDTH) - leftWidth;

	BView* leftPaneView = FindView("LeftPaneScroller");
	if (leftPaneView != NULL) {
		leftPaneView->MoveTo(bounds.left, bounds.top);
		leftPaneView->ResizeTo(leftWidth - 1, bounds.Height());
	}

	BView* rightPaneView = FindView("RightPaneScroller");
	if (rightPaneView != NULL) {
		rightPaneView->MoveTo(bounds.left + leftWidth + PANE_SPLITTER_WIDTH, bounds.top);
		rightPaneView->ResizeTo(rightWidth - 1, bounds.Height());
	}
}


void
TextDiffView::Draw(BRect updateRect)
{
	rgb_color oldHighColor = HighColor();
	float oldPenSize = PenSize();

	BRect bounds = Bounds();
	float leftWidth = floor((bounds.Width() + 1 - be_control_look->GetScrollBarWidth(B_HORIZONTAL)
		- PANE_SPLITTER_WIDTH) / 2);

	// Draw the body of the splitter
	BRect paneRect = bounds;
	bounds.left = leftWidth + 1;
	bounds.right = leftWidth + PANE_SPLITTER_WIDTH - 2;
	FillRect(paneRect, B_SOLID_LOW);

	if (updateRect.left <= leftWidth + 1) {
		SetPenSize(0);
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
		StrokeLine(
			BPoint(leftWidth, updateRect.top), BPoint(leftWidth, updateRect.bottom), B_SOLID_HIGH);
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_1_TINT));
		StrokeLine(BPoint(leftWidth + 1, updateRect.top), BPoint(leftWidth + 1, updateRect.bottom),
			B_SOLID_HIGH);
	}
	if (updateRect.right >= leftWidth + PANE_SPLITTER_WIDTH - 2) {
		SetPenSize(0);
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
		StrokeLine(BPoint(leftWidth + PANE_SPLITTER_WIDTH - 2, updateRect.top),
			BPoint(leftWidth + PANE_SPLITTER_WIDTH - 2, updateRect.bottom), B_SOLID_HIGH);
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
		StrokeLine(BPoint(leftWidth + PANE_SPLITTER_WIDTH - 1, updateRect.top),
			BPoint(leftWidth + PANE_SPLITTER_WIDTH - 1, updateRect.bottom), B_SOLID_HIGH);
	}

	SetPenSize(oldPenSize);
	SetHighColor(oldHighColor);
}


void
TextDiffView::paneVScrolled(float y, TextDiffView::PaneIndex fromPaneIndex)
{
	if (isPanesVScrolling)
		return;
	isPanesVScrolling = true;

	int index;
	for (index = 0; index < PaneMAX; index++) {
		if (index == fromPaneIndex)
			continue;

		const char* viewName;
		switch (index) {
			case LeftPane:
				viewName = "LeftPane";
				break;
			case RightPane:
				viewName = "RightPane";
				break;
			default:
				viewName = NULL;
				break;
		}

		if (viewName != NULL) {
			BView* pane = FindView(viewName);
			if (pane != NULL) {
				BRect bounds = pane->Bounds();
				pane->ScrollTo(bounds.left, y);
			}
		}
	}

	isPanesVScrolling = false;
}


void
TextDiffView::makeFocusToPane(TextDiffView::PaneIndex /* paneIndex */)
{
	BView* rightPaneView = FindView("RightPane");
	if (rightPaneView != NULL)
		rightPaneView->MakeFocus();
}


class LineSeparatedSequences : public Sequences
{
private:
	const LineSeparatedText* leftText;
	const LineSeparatedText* rightText;

public:
	LineSeparatedSequences(const LineSeparatedText* leftText, const LineSeparatedText* rightText)
	{
		this->leftText = leftText;
		this->rightText = rightText;
	}

	~LineSeparatedSequences() {}

	virtual int GetLength(int seqNo) const
	{
		switch (seqNo) {
			case 0:
				return leftText->GetLineCount();
				break;
			case 1:
				return rightText->GetLineCount();
				break;
			default:
				return 0;
		}
	}

	virtual bool IsEqual(int index0, int index1) const
	{
		return leftText->GetLineAt(index0) == rightText->GetLineAt(index1);
	}
};


void
TextDiffView::ExecuteDiff(
	const BPath& pathLeft, const BPath& pathRight)
{
	textData[LeftPane].Unload();
	textData[RightPane].Unload();
	lineInfos.clear();

	try {
		textData[LeftPane].Load(pathLeft);
		textData[RightPane].Load(pathRight);

		LineSeparatedSequences seqs(&textData[LeftPane], &textData[RightPane]);
		NPDiff diffEngine;
		diffEngine.Detect(&seqs);

		int index;
		for (index = 0;; index++) {
			const DiffOperation* diffOperation = diffEngine.GetOperationAt(index);
			if (NULL == diffOperation)
				break;

			LineInfo line;
			line.op = diffOperation->op;

			int count, maxCount;
			switch (diffOperation->op) {
				case DiffOperation::Inserted:
					line.textIndex[LeftPane] = -1;
					line.textIndex[RightPane] = diffOperation->from1;
					maxCount = diffOperation->count1;
					for (count = 0; count < maxCount; count++) {
						lineInfos.push_back(line);
						line.textIndex[RightPane]++;
					}
					break;

				case DiffOperation::Modified:
					line.textIndex[LeftPane] = diffOperation->from0;
					line.textIndex[RightPane] = diffOperation->from1;
					maxCount = (diffOperation->count0 > diffOperation->count1)
						? diffOperation->count0
						: diffOperation->count1;
					for (count = 0; count < maxCount; count++) {
						lineInfos.push_back(line);
						if (count + 1 < diffOperation->count0)
							line.textIndex[LeftPane]++;
						else
							line.textIndex[LeftPane] = -1;
						if (count + 1 < diffOperation->count1)
							line.textIndex[RightPane]++;
						else
							line.textIndex[RightPane] = -1;
					}
					break;

				case DiffOperation::Deleted:
					line.textIndex[LeftPane] = diffOperation->from0;
					line.textIndex[RightPane] = -1;
					maxCount = diffOperation->count0;
					for (count = 0; count < maxCount; count++) {
						lineInfos.push_back(line);
						line.textIndex[LeftPane]++;
					}
					break;

				case DiffOperation::NotChanged:
					line.textIndex[LeftPane] = diffOperation->from0;
					line.textIndex[RightPane] = diffOperation->from1;
					maxCount = diffOperation->count0;
					for (count = 0; count < maxCount; count++) {
						lineInfos.push_back(line);
						line.textIndex[LeftPane]++;
						line.textIndex[RightPane]++;
					}
					break;
			}
		}
	} catch (Exception* ex) {
		ex->Delete();
	}

	DiffPaneView* leftPaneView = dynamic_cast<DiffPaneView*>(FindView("LeftPane"));
	if (leftPaneView != NULL)
		leftPaneView->DataChanged();
	DiffPaneView* rightPaneView = dynamic_cast<DiffPaneView*>(FindView("RightPane"));
	if (rightPaneView != NULL)
		rightPaneView->DataChanged();

	makeFocusToPane(LeftPane);
}


TextDiffView::DiffPaneView::DiffPaneView(BRect frame, const char* name, uint32 resizingMode)
	:
	BView(frame, name, resizingMode, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	textDiffView = NULL;
	paneIndex = TextDiffView::InvalidPane;
	scroller = NULL;
	dataHeight = -1;
	tabUnit = -1;

	// Don't let the app server erase the view.
	// We do all drawing ourselves, so it is not necessary and only causes flickering
	SetViewColor(B_TRANSPARENT_COLOR);
}


TextDiffView::DiffPaneView::~DiffPaneView()
{
}


void
TextDiffView::DiffPaneView::DataChanged()
{
	dataHeight = -1;
	ScrollTo(BPoint(0, 0));
	Invalidate();
}


void
TextDiffView::DiffPaneView::recalcLayout()
{
	dataHeight = -1;

	BRect bounds = Bounds();
	float boundsHeight = bounds.Height() + 1;
	float height = getDataHeight();
	if (height < boundsHeight)
		height = boundsHeight;

	if (bounds.bottom >= height)
		ScrollTo(BPoint(bounds.left, height - boundsHeight));
	else
		adjustScrollBar();
}


void
TextDiffView::DiffPaneView::adjustScrollBar()
{
	if (NULL == scroller)
		return;

	BRect bounds = Bounds();

	BScrollBar* verticalBar = scroller->ScrollBar(B_VERTICAL);
	if (verticalBar != NULL) {
		float boundsHeight = bounds.Height() + 1;
		float height = getDataHeight();
		if (height < boundsHeight)
			height = boundsHeight;
		verticalBar->SetRange(0, height - boundsHeight);
		verticalBar->SetProportion(boundsHeight / height);

		BFont font;
		GetFont(&font);
		font_height fh;
		font.GetHeight(&fh);
		float lineHeight = static_cast<float>(ceil(fh.ascent + fh.descent + fh.leading));
		verticalBar->SetSteps(lineHeight, boundsHeight);
	}

	BScrollBar* horizontalBar = scroller->ScrollBar(B_HORIZONTAL);
	if (horizontalBar != NULL) {
		float boundsWidth = bounds.Width() + 1;
		horizontalBar->SetRange(0, HORIZONTAL_SCROLL_MAX);
		horizontalBar->SetProportion(boundsWidth / (HORIZONTAL_SCROLL_MAX + boundsWidth));
		horizontalBar->SetSteps(HORIZONTAL_SCROLL_MINSTEPS, boundsWidth);
	}
}


float
TextDiffView::DiffPaneView::getDataHeight()
{
	if (dataHeight < 0) {
		if (textDiffView != NULL) {
			BFont font;
			GetFont(&font);

			font_height fh;
			font.GetHeight(&fh);
			float lineHeight = static_cast<float>(ceil(fh.ascent + fh.descent + fh.leading));

			dataHeight = textDiffView->lineInfos.size() * lineHeight;
		}
	}

	return dataHeight;
}


void
TextDiffView::DiffPaneView::TargetedByScrollView(BScrollView* scroller)
{
	this->scroller = scroller;
}


void
TextDiffView::DiffPaneView::Draw(BRect updateRect)
{
	if (NULL == textDiffView || TextDiffView::InvalidPane == paneIndex)
		return;

	SetLowUIColor(B_DOCUMENT_BACKGROUND_COLOR);
	SetHighUIColor(B_DOCUMENT_TEXT_COLOR);
	FillRect(Bounds(), B_SOLID_LOW);

	BFont font;
	GetFont(&font);

	font_height fh;
	font.GetHeight(&fh);
	float lineHeight = static_cast<float>(ceil(fh.ascent + fh.descent + fh.leading));

	int lineBegin = static_cast<int>(floor(updateRect.top / lineHeight));
	if (lineBegin < 0)
		lineBegin = 0;
	int lineEnd = static_cast<int>(floor((updateRect.bottom + 1) / lineHeight)) + 1;
	if (static_cast<unsigned int>(lineEnd) > textDiffView->lineInfos.size())
		lineEnd = textDiffView->lineInfos.size();
	int line;
	for (line = lineBegin; line < lineEnd; line++) {
		rgb_color oldLowColor = LowColor();
		const LineInfo& linfo = textDiffView->lineInfos[line];

		rgb_color bkColor;
		bool isDrawBackground = false;
		switch (linfo.op) {
			case DiffOperation::Inserted:
			{
				if (paneIndex == TextDiffView::RightPane)
					bkColor = colorInserted;
				else
					bkColor = colorEmpty;
				isDrawBackground = true;
			} break;

			case DiffOperation::Deleted:
			{
				if (paneIndex == TextDiffView::LeftPane)
					bkColor = colorDeleted;
				else
					bkColor = colorEmpty;
				isDrawBackground = true;
			} break;

			case DiffOperation::Modified:
			{
				bkColor = colorModified;
				isDrawBackground = true;
			} break;

			default:
				isDrawBackground = false;
				break;
		}
		if (isDrawBackground) {
			SetLowColor(bkColor);
			FillRect(BRect(updateRect.left, lineHeight * line, updateRect.right,
				lineHeight * (line + 1) - 1), B_SOLID_LOW);
		}

		if (linfo.textIndex[paneIndex] >= 0) {
			const Substring& paneText
				= textDiffView->textData[paneIndex].GetLineAt(linfo.textIndex[paneIndex]);
			drawText(font, paneText, lineHeight * line + fh.ascent);
		}

		SetLowColor(oldLowColor);
	}
}


void
TextDiffView::DiffPaneView::drawText(const BFont& font, const Substring& text, float baseLine)
{
	float left = 0;
	const char* subTextBegin = text.Begin();
	const char* end = text.End();
	const char* ptr;
	for (ptr = subTextBegin; ptr < end; ptr++) {
		if ('\t' == *ptr || '\r' == *ptr || '\n' == *ptr) {
			int count = ptr - subTextBegin;
			if (count > 0) {
				DrawString(subTextBegin, ptr - subTextBegin, BPoint(left, baseLine));
				left += font.StringWidth(subTextBegin, ptr - subTextBegin);
			}
			subTextBegin = ptr + 1;
		}

		if ('\t' == *ptr) {
			if (tabUnit < 0) {
				tabUnit = font.StringWidth(FONT_SAMPLE, FONT_SAMPLE_LENGTH) / FONT_SAMPLE_LENGTH
					* tabChars;
			}
			left = (floor(left / tabUnit) + 1) * tabUnit;
		}
	}
	if (subTextBegin < end)
		DrawString(subTextBegin, end - subTextBegin, BPoint(left, baseLine));
}


void
TextDiffView::DiffPaneView::ScrollTo(BPoint point)
{
	BView::ScrollTo(point);

	adjustScrollBar();
	if (textDiffView != NULL)
		textDiffView->paneVScrolled(point.y, paneIndex);
}


void
TextDiffView::DiffPaneView::MouseDown(BPoint point)
{
	BView::MouseDown(point);

	if (textDiffView != NULL)
		textDiffView->makeFocusToPane(paneIndex);
}


void
TextDiffView::DiffPaneView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);

	recalcLayout();
}


void
TextDiffView::DiffPaneView::SetFont(const BFont* font, uint32 properties /* = B_FONT_ALL */)
{
	BView::SetFont(font, properties);

	tabUnit = -1;

	recalcLayout();
}
