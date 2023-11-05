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
 *	@file		TextDiffView.h
 *	@brief		TextDiffView クラスの定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-25 Created
 */
#ifndef TEXTDIFFVIEW_H__INCLUDED
#define TEXTDIFFVIEW_H__INCLUDED


#include <View.h>

#include <vector>

#include "LineSeparatedText.h"
#include "NPDiff.h"

class BPath;

/**
 *	@brief	テキストの差分を表示するビュー
 */
class TextDiffView : public BView {
public:
						TextDiffView(BRect frame, const char* name, uint32 resizingMode);
	virtual				~TextDiffView();

			void		Initialize();
			void		ExecuteDiff(const BPath& pathLeft, const BPath& pathRight,
							const char* labelLeft, const char* labelRight);

public:
	virtual	void		FrameResized(float width, float height);
	virtual	void		Draw(BRect updateRect);

private:
	/// ペインのインデックス
	enum PaneIndex {
		InvalidPane = -1,	///< 無効

		LeftPane = 0,		///< 左ペイン
		RightPane,			///< 右ペイン

		PaneMAX				///< ペインの最大数
	};

private:
			void		recalcLayout();
			void		paneVScrolled(float y, TextDiffView::PaneIndex fromPaneIndex);
			void		makeFocusToPane(TextDiffView::PaneIndex paneIndex);

private:
	/// 左または右のペイン 1 つ分のビュー
	class DiffPaneView : public BView {
	public:
							DiffPaneView(BRect frame, const char* name, uint32 resizingMode);
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
		virtual void		MouseDown(BPoint point);
		virtual	void		FrameResized(float width, float height);
		virtual void		SetFont(const BFont* font, uint32 properties = B_FONT_ALL);

	private:
				void		recalcLayout();
				void		adjustScrollBar();
				float		getDataHeight();
				void		drawText(const BFont& font, const Substring& text, float baseLine);

	private:
		TextDiffView*		textDiffView;	///< 親の TextDiffView
		TextDiffView::PaneIndex	paneIndex;	///< このペインのインデックス
		BScrollView*		scroller;		///< コンテナのスクロールビュー
		float				dataHeight;		///< データ領域の高さ（負の値ならまだ取得していないことを示す）
		float				tabUnit;		///< タブ 1 つ分のピクセル幅（負の値ならまだ取得していないことを示す）
	};
	friend class DiffPaneView;

	/// 1行分の行情報
	struct LineInfo {
		int32 textIndex[PaneMAX];		///< テキストの行インデックス（なければ -1）
		DiffOperation::Operator op;		///< この行の Diff 操作
	};
	typedef std::vector<LineInfo> LineInfoVector;

private:
		LineSeparatedText	textData[PaneMAX];	///< テキストのデータ
		LineInfoVector		lineInfos;			///< 行情報の配列
		bool				isPanesVScrolling;	///< 各ペインをスクロールさせている途中なら true
};

#endif // TEXTDIFFVIEW_H__INCLUDED
