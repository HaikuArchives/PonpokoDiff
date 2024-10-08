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

#include "DiffView.h"

#include <math.h>

#include "CommandIDs.h"
#include "Exception.h"
#include "TextFileFilter.h"

#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <ScrollBar.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <Window.h>

#include <cstdio>

static const char FONT_SAMPLE[] = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int FONT_SAMPLE_LENGTH = sizeof(FONT_SAMPLE) - 1;
static const int TAB_CHARS = 4;

enum system_theme {
	LIGHT = 0,
	DARK = 1
};

rgb_color colorInserted[] = {
	{163, 255, 180, 255},
	{0, 90, 20, 255}
};
static const rgb_color colorDeleted[] = {
	{255, 221, 191, 255},
	{128, 60, 0, 255}
};
static const rgb_color colorModified[] = {
	{255, 255, 191, 255},
	{110, 110, 0, 255}
};

// Added until beta5, when functions like IsLight() become available
static inline uint8
perceptual_brightness(rgb_color color)
{
	// From http://alienryderflex.com/hsp.html
	// Useful in particular to decide if the color is "light" or "dark"
	// by checking if the perceptual brightness is > 127.
	int r = color.red;
	int g = color.green;
	int b = color.blue;

	return (uint8)roundf(sqrtf(
		0.299f * r * r + 0.587f * g * g + 0.114 * b * b));
}


DiffView::DiffView(const char* name)
	:
	BView("name", B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_SUPPORTS_LAYOUT)
{
	fIsPanesScrolling = false;

	_Initialize();
}


DiffView::~DiffView()
{
}


void
DiffView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SIMPLE_DATA:
		{
			if (message->WasDropped()) {
				int validFileCount = 0;
				BPath paths[2];
				BEntry entry;
				entry_ref ref;
				for (int refCount = 0; message->FindRef("refs", refCount, &ref)
					== B_NO_ERROR; refCount++) {
					// Only allow text files, XML and catkeys
					entry.SetTo(&ref, true); // traverse links
					TextFileFilter filter;
					if (filter.IsValid(&ref, &entry)) {
						paths[validFileCount].SetTo(&entry);
						validFileCount++;
					}
					// Only grab the first two valid files
					if (validFileCount == 2)
						break;
				}
				if (validFileCount == 0)
					break;

				BMessage msg(MSG_FILE_DROPPED);
				if (validFileCount == 2) {
					msg.AddString("leftpath", paths[0].Path());
					msg.AddString("rightpath", paths[1].Path());
				} else {
					// Dropped on the left or right side
					BPoint dropPoint = message->DropPoint();
					ConvertFromScreen(&dropPoint);
					BView* leftPaneView = FindView("LeftPane");

					if (leftPaneView != NULL) {
						BRect rect = leftPaneView->Bounds();
						if (rect.Contains(dropPoint))
							msg.AddString("leftpath", paths[0].Path());
						else
							msg.AddString("rightpath", paths[0].Path());
					}
				}
				Window()->PostMessage(&msg);
			}
		} break;

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
DiffView::_Initialize()
{
	DiffPaneView* leftPaneView = new DiffPaneView("LeftPane");
	leftPaneView->SetDiffView(this);
	leftPaneView->SetPaneIndex(LEFT_PANE);

	BScrollView* leftView = new BScrollView("LeftPaneScroller", leftPaneView,
		B_FRAME_EVENTS | B_SUPPORTS_LAYOUT, true, true, B_PLAIN_BORDER);
	BScrollBar* vBar = leftView->ScrollBar(B_VERTICAL);
	vBar->SetExplicitSize(BSize(0, B_SIZE_UNSET));

	DiffPaneView* rightPaneView = new DiffPaneView("RightPane");
	rightPaneView->SetDiffView(this);
	rightPaneView->SetPaneIndex(RIGHT_PANE);

	BScrollView* rightView = new BScrollView("RightPaneScroller", rightPaneView,
		B_FRAME_EVENTS | B_SUPPORTS_LAYOUT, true, true, B_PLAIN_BORDER);

	// Don't let the app server erase the view.
	// We do all drawing ourselves, so it is not necessary and only causes flickering
	SetViewColor(B_TRANSPARENT_COLOR);
	// The splitter in the middle will be drawn with the low color
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);

	BSeparatorView* separator = new BSeparatorView(B_VERTICAL, B_FANCY_BORDER);
	separator->SetExplicitMinSize(BSize(2, B_SIZE_UNSET));

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.SetInsets(-1, -1, 0, 0)
		.Add(leftView)
		.Add(separator)
		.Add(rightView)
		.End();
}


void
DiffView::_PaneScrolled(float x, float y, DiffView::PaneIndex fromPaneIndex)
{
	if (fIsPanesScrolling)
		return;
	fIsPanesScrolling = true;

	int index;
	for (index = 0; index < PaneMAX; index++) {
		const char* viewName;
		switch (index) {
			case LEFT_PANE:
				viewName = "LeftPane";
				break;
			case RIGHT_PANE:
				viewName = "RightPane";
				break;
			default:
				viewName = NULL;
				break;
		}

		if (viewName != NULL) {
			BView* pane = FindView(viewName);
			if (pane != NULL)
				pane->ScrollTo(x, y);
		}
	}

	fIsPanesScrolling = false;
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
DiffView::ExecuteDiff(BPath pathLeft, BPath pathRight)
{
	fTextData[LEFT_PANE].Unload();
	fTextData[RIGHT_PANE].Unload();
	fLineInfos.clear();

	fIdentical = true;
	try {
		fTextData[LEFT_PANE].Load(pathLeft);
		fTextData[RIGHT_PANE].Load(pathRight);

		LineSeparatedSequences seqs(&fTextData[LEFT_PANE], &fTextData[RIGHT_PANE]);
		NPDiff diffEngine;
		diffEngine.Detect(&seqs);

		int index;
		for (index = 0;; index++) {
			const DiffOperation* diffOperation = diffEngine.GetOperationAt(index);
			if (diffOperation == NULL)
				break;

			LineInfo line;
			line.op = diffOperation->op;

			int count, maxCount;
			switch (diffOperation->op) {
				case DiffOperation::Inserted:
					line.textIndex[LEFT_PANE] = -1;
					line.textIndex[RIGHT_PANE] = diffOperation->from1;
					maxCount = diffOperation->count1;
					for (count = 0; count < maxCount; count++) {
						fLineInfos.push_back(line);
						line.textIndex[RIGHT_PANE]++;
					}
					fIdentical = false;
					break;

				case DiffOperation::Modified:
					line.textIndex[LEFT_PANE] = diffOperation->from0;
					line.textIndex[RIGHT_PANE] = diffOperation->from1;
					maxCount = (diffOperation->count0 > diffOperation->count1)
						? diffOperation->count0
						: diffOperation->count1;
					for (count = 0; count < maxCount; count++) {
						fLineInfos.push_back(line);
						if (count + 1 < diffOperation->count0)
							line.textIndex[LEFT_PANE]++;
						else
							line.textIndex[LEFT_PANE] = -1;
						if (count + 1 < diffOperation->count1)
							line.textIndex[RIGHT_PANE]++;
						else
							line.textIndex[RIGHT_PANE] = -1;
					}
					fIdentical = false;
					break;

				case DiffOperation::Deleted:
					line.textIndex[LEFT_PANE] = diffOperation->from0;
					line.textIndex[RIGHT_PANE] = -1;
					maxCount = diffOperation->count0;
					for (count = 0; count < maxCount; count++) {
						fLineInfos.push_back(line);
						line.textIndex[LEFT_PANE]++;
					}
					fIdentical = false;
					break;

				case DiffOperation::NotChanged:
					line.textIndex[LEFT_PANE] = diffOperation->from0;
					line.textIndex[RIGHT_PANE] = diffOperation->from1;
					maxCount = diffOperation->count0;
					for (count = 0; count < maxCount; count++) {
						fLineInfos.push_back(line);
						line.textIndex[LEFT_PANE]++;
						line.textIndex[RIGHT_PANE]++;
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
}


DiffView::DiffPaneView::DiffPaneView(const char* name)
	:
	BView(BRect(), name, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	fDiffView = NULL;
	fPaneIndex = DiffView::InvalidPane;
	fScroller = NULL;
	fDataHeight = -1;
	fDataWidth = -1;
	fTabUnit = -1;

	// Don't let the app server erase the view.
	// We do all drawing ourselves, so it is not necessary and only causes flickering
	SetViewColor(B_TRANSPARENT_COLOR);
}


DiffView::DiffPaneView::~DiffPaneView()
{
}


void
DiffView::DiffPaneView::DataChanged()
{
	fDataHeight = -1;
	fDataWidth = -1;
	ScrollTo(BPoint(0, 0));
	Invalidate();
	_AdjustScrollBar();
}


void
DiffView::DiffPaneView::_RecalcLayout()
{
	fDataHeight = -1;

	BRect bounds = Bounds();
	float boundsHeight = bounds.Height() + 1;
	float height = _GetDataHeight();
	if (height < boundsHeight)
		height = boundsHeight;

	if (bounds.bottom >= height)
		ScrollTo(BPoint(bounds.left, height - boundsHeight));
	else
		_AdjustScrollBar();
}


void
DiffView::DiffPaneView::_AdjustScrollBar()
{
	if (fScroller == NULL)
		return;

	BRect bounds = Bounds();

	BScrollBar* verticalBar = fScroller->ScrollBar(B_VERTICAL);
	if (verticalBar != NULL) {
		float boundsHeight = bounds.Height() + 1;
		float height = _GetDataHeight();
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

	BScrollBar* horizontalBar = fScroller->ScrollBar(B_HORIZONTAL);
	if (horizontalBar != NULL) {
		float boundsWidth = bounds.Width();
		float dataWidth = _GetDataWidth();
		float range = dataWidth - boundsWidth;
		if (range > 0) {
			horizontalBar->SetRange(0, range);
			horizontalBar->SetProportion((boundsWidth + 1) / (dataWidth + 1));
			horizontalBar->SetSteps(8, boundsWidth + 1);
		} else {
			horizontalBar->SetRange(0, 0);
			horizontalBar->SetProportion(1);
		}
	}
}


float
DiffView::DiffPaneView::_GetDataHeight()
{
	if (fDataHeight < 0) {
		if (fDiffView != NULL) {
			BFont font;
			GetFont(&font);

			font_height fh;
			font.GetHeight(&fh);
			float lineHeight = static_cast<float>(ceil(fh.ascent + fh.descent + fh.leading));

			fDataHeight = fDiffView->fLineInfos.size() * lineHeight;
		}
	}

	return fDataHeight;
}


float
DiffView::DiffPaneView::_GetDataWidth()
{
	if ((fDataWidth >= 0) || (fDiffView == NULL))
		return fDataWidth;

	int lineEnd = fDiffView->fLineInfos.size();
	int line;
	for (line = 0; line < lineEnd; line++) {
		const LineInfo& linfo = fDiffView->fLineInfos[line];
		if (linfo.textIndex[fPaneIndex] >= 0) {
			const Substring& text
				= fDiffView->fTextData[fPaneIndex].GetLineAt(linfo.textIndex[fPaneIndex]);
			BFont font;
			GetFont(&font);
			float left = 0;
			const char* subTextBegin = text.Begin();
			const char* end = text.End();
			const char* ptr;
			for (ptr = subTextBegin; ptr < end; ptr++) {
				if ('\t' == *ptr || '\r' == *ptr || '\n' == *ptr) {
					int count = ptr - subTextBegin;
					if (count > 0)
						left += font.StringWidth(subTextBegin, ptr - subTextBegin);
					subTextBegin = ptr + 1;
				}

				if ('\t' == *ptr) {
					if (fTabUnit < 0) {
						fTabUnit = font.StringWidth(FONT_SAMPLE, FONT_SAMPLE_LENGTH)
							/ FONT_SAMPLE_LENGTH * TAB_CHARS;
					}
					left = (floor(left / fTabUnit) + 1) * fTabUnit;
				}
				fDataWidth = std::max(fDataWidth, left);
			}
		}
	}

	return fDataWidth;
}


void
DiffView::DiffPaneView::TargetedByScrollView(BScrollView* fScroller)
{
	this->fScroller = fScroller;
}


void
DiffView::DiffPaneView::Draw(BRect updateRect)
{
	if (fDiffView == NULL || DiffView::InvalidPane == fPaneIndex)
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
	if (static_cast<unsigned int>(lineEnd) > fDiffView->fLineInfos.size())
		lineEnd = fDiffView->fLineInfos.size();
	int line;
	for (line = lineBegin; line < lineEnd; line++) {
		rgb_color oldLowColor = LowColor();
		const LineInfo& linfo = fDiffView->fLineInfos[line];

		rgb_color bkColor;
		int brightness = perceptual_brightness(ui_color(B_DOCUMENT_TEXT_COLOR));
		system_theme theme;
		theme = brightness > 127 ? DARK : LIGHT;

		bool isDrawBackground = false;
		switch (linfo.op) {
			case DiffOperation::Inserted:
			{
				if (fPaneIndex == DiffView::RIGHT_PANE)
					bkColor = colorInserted[theme];
				else
					bkColor = ui_color(B_PANEL_BACKGROUND_COLOR);
				isDrawBackground = true;
			} break;

			case DiffOperation::Deleted:
			{
				if (fPaneIndex == DiffView::LEFT_PANE)
					bkColor = colorDeleted[theme];
				else
					bkColor = ui_color(B_PANEL_BACKGROUND_COLOR);
				isDrawBackground = true;
			} break;

			case DiffOperation::Modified:
			{
				bkColor = colorModified[theme];
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

		if (linfo.textIndex[fPaneIndex] >= 0) {
			const Substring& paneText
				= fDiffView->fTextData[fPaneIndex].GetLineAt(linfo.textIndex[fPaneIndex]);
			_DrawText(font, paneText, lineHeight * line + fh.ascent);
		}
		SetLowColor(oldLowColor);
	}
}


void
DiffView::DiffPaneView::_DrawText(const BFont& font, const Substring& text, float baseLine)
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
			if (fTabUnit < 0) {
				fTabUnit = font.StringWidth(FONT_SAMPLE, FONT_SAMPLE_LENGTH) / FONT_SAMPLE_LENGTH
					* TAB_CHARS;
			}
			left = (floor(left / fTabUnit) + 1) * fTabUnit;
		}
	}

	if (subTextBegin < end)
		DrawString(subTextBegin, end - subTextBegin, BPoint(left, baseLine));
}


void
DiffView::DiffPaneView::ScrollTo(BPoint point)
{
	BView::ScrollTo(point);

	if (fDiffView != NULL)
		fDiffView->_PaneScrolled(point.x, point.y, fPaneIndex);
}


void
DiffView::DiffPaneView::MouseDown(BPoint where)
{
	BView::MouseDown(where);

	BMessage* message = Window()->CurrentMessage();

    int32 mods = 0, clicks = 0, buttons=0;
    message->FindInt32("modifiers", &mods);
    message->FindInt32("clicks", &clicks);
    message->FindInt32("buttons", &buttons);

	// double click handling
	if (clicks % 2 == 0) {
		if (buttons == B_PRIMARY_MOUSE_BUTTON) {
			BMessage msg;
			if (mods & B_CONTROL_KEY)
				msg.what = MSG_OPEN_LOCATION;
			else
				msg.what = MSG_FILE_LAUNCH;
			msg.AddInt32("pane", fPaneIndex);
			Window()->PostMessage(&msg);
		}
	}
}


void
DiffView::DiffPaneView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);

	_RecalcLayout();
}


void
DiffView::DiffPaneView::SetFont(const BFont* font, uint32 properties /* = B_FONT_ALL */)
{
	BView::SetFont(font, properties);

	fTabUnit = -1;

	_RecalcLayout();
}
