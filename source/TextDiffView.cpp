/*
 * PonpokoDiff
 *
 * Copyright (c) 2008 PonpokoDiff Project Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 *	@file		TextDiffView.cpp
 *	@brief		TextDiffView クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-25 Created
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

static const int tabChars = 4; /// タブの文字数

/// ペインを分割している部分の幅
#define PANE_SPLITTER_WIDTH 6

/// 水平方向にどこまでスクロールするか
#define HORIZONTAL_SCROLL_MAX 4000

/// 水平方向の小さいステップ
#define HORIZONTAL_SCROLL_MINSTEPS 8

/**
 *	@brief	コンストラクタ
 *	@param[in]	frame			ビューの矩形
 *	@param[in]	name			ビューの名前
 *	@param[in]	resizingMode	ビューのリサイズモード
 */
TextDiffView::TextDiffView(BRect frame, const char* name, uint32 resizingMode)
	:
	BView(frame, name, resizingMode, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	isPanesVScrolling = false;
}

/**
 *	@brief	デストラクタ
 */
TextDiffView::~TextDiffView()
{
}

/**
 *	@brief	初期化
 */
void
TextDiffView::Initialize()
{
	BRect bounds = Bounds();
	float leftWidth = floor((bounds.Width() + 1 - be_control_look->GetScrollBarWidth(B_HORIZONTAL)
		- PANE_SPLITTER_WIDTH) / 2);

	// 左ペインを作成
	BRect leftFrame = BRect(bounds.left, bounds.top, bounds.left + leftWidth - 1,
		bounds.bottom - be_control_look->GetScrollBarWidth(B_HORIZONTAL));
	DiffPaneView* leftPaneView = new DiffPaneView(leftFrame, "LeftPane", B_FOLLOW_ALL_SIDES);
	leftPaneView->SetTextDiffView(this);
	leftPaneView->SetPaneIndex(LeftPane);

	BScrollView* leftView = new BScrollView("LeftPaneScroller", leftPaneView, B_FOLLOW_NONE,
		B_FRAME_EVENTS, true, false, B_NO_BORDER);
	AddChild(leftView);

	// 右ペインを作成
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

	// 念のためレイアウトを調整
	recalcLayout();

	leftPaneView->DataChanged();
	rightPaneView->DataChanged();
}

/**
 *	@brief フレームのサイズが変化したときに呼び出されます。
 */
void
TextDiffView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
	recalcLayout();
}

/**
 *	@brief	レイアウトを調整します。
 */
void
TextDiffView::recalcLayout()
{
	BRect bounds = Bounds();
	float leftWidth = floor((bounds.Width() + 1 - be_control_look->GetScrollBarWidth(B_HORIZONTAL)
		- PANE_SPLITTER_WIDTH) / 2);
	float rightWidth = (bounds.Width() + 1 - PANE_SPLITTER_WIDTH) - leftWidth;

	// 左ペイン
	BView* leftPaneView = FindView("LeftPaneScroller");
	if (NULL != leftPaneView) {
		leftPaneView->MoveTo(bounds.left, bounds.top);
		leftPaneView->ResizeTo(leftWidth - 1, bounds.Height());
	}

	// 右ペイン
	BView* rightPaneView = FindView("RightPaneScroller");
	if (NULL != rightPaneView) {
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

/**
 *	@brief	ペインが縦方向にスクロールしたら呼び出します。
 *	@param[in]	y				スクロール後の y 座標
 *	@param[in]	fromPaneIndex	この関数を呼び出しているペインのインデックス
 */
void
TextDiffView::paneVScrolled(float y, TextDiffView::PaneIndex fromPaneIndex)
{
	// 再入を防ぐ
	if (isPanesVScrolling)
		return;
	isPanesVScrolling = true;

	// 他のペインもスクロールさせる
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

		if (NULL != viewName) {
			BView* pane = FindView(viewName);
			if (NULL != pane) {
				BRect bounds = pane->Bounds();
				pane->ScrollTo(bounds.left, y);
			}
		}
	}

	isPanesVScrolling = false;
}

/**
 *	@brief	Diff ペインにフォーカスを与えます。
 *	@param[in]	paneIndex	フォーカスを与えるペイン
 */
void
TextDiffView::makeFocusToPane(TextDiffView::PaneIndex /* paneIndex */)
{
	// ホイールイベントのために常に右ペインにフォーカスを与えます。
	// 必要ならここでフォーカスのあるはずのペインを記憶してもいいかも
	BView* rightPaneView = FindView("RightPane");
	if (NULL != rightPaneView)
		rightPaneView->MakeFocus();
}

/**
 *	@brief	NPDiff オブジェクトにセットする行単位の符号列クラス
 */
class LineSeparatedSequences : public Sequences
{
private:
	const LineSeparatedText* leftText; ///< 左側テキスト
	const LineSeparatedText* rightText; ///< 右側テキスト

public:
	LineSeparatedSequences(const LineSeparatedText* leftText, const LineSeparatedText* rightText)
	{
		this->leftText = leftText;
		this->rightText = rightText;
	}

	~LineSeparatedSequences() {}

	/**
	 *	@brief	指定した符号列の長さを得ます。
	 *	@param[in]	seqNo	0 または 1 を指定します。
	 *	@return	符号列の長さを返します。
	 */
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

	/**
	 *	@brief	指定したインデックスの符号が一致するかどうかを調べます。
	 *	@param[in]	index0	符号列 0 のインデックス (0 を起点とします)
	 *	@param[in]	index1	符号列 1 のインデックス (0 を起点とします)
	 *	@return	一致するなら true。
	 */
	virtual bool IsEqual(int index0, int index1) const
	{
		return leftText->GetLineAt(index0) == rightText->GetLineAt(index1);
	}
};

/**
 *	@brief	Diff を実行します。
 *	@param[in]	pathLeft	左ペインに表示するファイルのパス
 *	@param[in]	pathRight	右ペインに表示するファイルのパス
 */
void
TextDiffView::ExecuteDiff(
	const BPath& pathLeft, const BPath& pathRight)
{
	// 初期化
	textData[LeftPane].Unload();
	textData[RightPane].Unload();
	lineInfos.clear();

	try {
		// 各ファイルを読み込み
		textData[LeftPane].Load(pathLeft);
		textData[RightPane].Load(pathRight);

		// diff 実行
		LineSeparatedSequences seqs(&textData[LeftPane], &textData[RightPane]);
		NPDiff diffEngine;
		diffEngine.Detect(&seqs);

		// diff 結果から行情報を作成
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
		// TODO: なんかメッセージ出す
		ex->Delete();
	}

	// ペインを調整
	DiffPaneView* leftPaneView = dynamic_cast<DiffPaneView*>(FindView("LeftPane"));
	if (NULL != leftPaneView)
		leftPaneView->DataChanged();
	DiffPaneView* rightPaneView = dynamic_cast<DiffPaneView*>(FindView("RightPane"));
	if (NULL != rightPaneView)
		rightPaneView->DataChanged();

	// 左ペインにフォーカスを与えておく
	makeFocusToPane(LeftPane);
}

/**
 *	@brief	コンストラクタ
 *	@param[in]	name	ビュー名
 */
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

/**
 *	@brief	デストラクタ
 */
TextDiffView::DiffPaneView::~DiffPaneView()
{
}

/**
 *	@brief	中に表示して入るデータが変わったら呼び出します。
 */
void
TextDiffView::DiffPaneView::DataChanged()
{
	dataHeight = -1; // 高さキャッシュをクリア
	ScrollTo(BPoint(0, 0));
	Invalidate();
}

/**
 * レイアウトを再計算します。
 */
void
TextDiffView::DiffPaneView::recalcLayout()
{
	dataHeight = -1; // 高さキャッシュをクリア

	BRect bounds = Bounds();
	float boundsHeight = bounds.Height() + 1;
	float height = getDataHeight();
	if (height < boundsHeight)
		height = boundsHeight;

	// サイズが広がったりした結果、縦方向に
	// スクロールできないところまでスクロールした状態になったら
	// バウンズ矩形を調整する。
	if (bounds.bottom >= height) {
		// この ScrollTo の中から adjustScrollBar が呼び出される
		ScrollTo(BPoint(bounds.left, height - boundsHeight));
	} else
		adjustScrollBar();
}

/**
 *	@brief	コンテナのスクロールバーを調整します。
 */
void
TextDiffView::DiffPaneView::adjustScrollBar()
{
	if (NULL == scroller)
		return;

	BRect bounds = Bounds();

	// 縦スクロールバーについての調整
	BScrollBar* verticalBar = scroller->ScrollBar(B_VERTICAL);
	if (NULL != verticalBar) {
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

	// 横スクロールバーについての調整
	BScrollBar* horizontalBar = scroller->ScrollBar(B_HORIZONTAL);
	if (NULL != horizontalBar) {
		float boundsWidth = bounds.Width() + 1;
		horizontalBar->SetRange(0, HORIZONTAL_SCROLL_MAX);
		horizontalBar->SetProportion(boundsWidth / (HORIZONTAL_SCROLL_MAX + boundsWidth));
		horizontalBar->SetSteps(HORIZONTAL_SCROLL_MINSTEPS, boundsWidth);
	}
}

/**
 *	@brief	データ領域の高さを求めます。
 *	@return	データ領域の高さ
 */
float
TextDiffView::DiffPaneView::getDataHeight()
{
	if (dataHeight < 0) {
		if (NULL != textDiffView) {
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

/**
 *	@brief	BScrollView のターゲットになった時に呼び出されます。
 *	@param[in]	scroller	BScrollView オブジェクト
 */
void
TextDiffView::DiffPaneView::TargetedByScrollView(BScrollView* scroller)
{
	this->scroller = scroller;
}

/**
 *	@brief	描画を行います。
 *	@param[in]	updateRect	描画をおこなうべき矩形
 */
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

		// 背景色描画
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

		// テキスト描画
		if (linfo.textIndex[paneIndex] >= 0) {
			const Substring& paneText
				= textDiffView->textData[paneIndex].GetLineAt(linfo.textIndex[paneIndex]);
			drawText(font, paneText, lineHeight * line + fh.ascent);
		}

		SetLowColor(oldLowColor);
	}
}

/**
 *	@brief	タブを意識してテキストを描画します。
 *	@param[in]	font		描画に使われるはずのフォント（幅を取得するためにのみ用います）
 *	@param[in]	text		描画テキスト
 *	@param[in]	baseLine	ベースライン
 */
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
			// タブ幅が計算されていなければ計算する
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

/**
 *	@brief	スクロールされたら呼び出されます。
 */
void
TextDiffView::DiffPaneView::ScrollTo(BPoint point)
{
	BView::ScrollTo(point);

	adjustScrollBar();
	if (NULL != textDiffView)
		textDiffView->paneVScrolled(point.y, paneIndex);
}

/**
 *	@brief	マウスボタンが押されたら呼び出されます。
 */
void
TextDiffView::DiffPaneView::MouseDown(BPoint point)
{
	BView::MouseDown(point);

	if (NULL != textDiffView)
		textDiffView->makeFocusToPane(paneIndex);
}

/**
 *	@brief フレームのサイズが変化したときに呼び出されます。
 */
void
TextDiffView::DiffPaneView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);

	recalcLayout();
}

/**
 *	@brief	フォントを変更するときに呼び出されます。
 */
void
TextDiffView::DiffPaneView::SetFont(const BFont* font, uint32 properties /* = B_FONT_ALL */)
{
	BView::SetFont(font, properties);

	// タブ幅のキャッシュをクリア
	tabUnit = -1;

	// 再レイアウト
	recalcLayout();
}
