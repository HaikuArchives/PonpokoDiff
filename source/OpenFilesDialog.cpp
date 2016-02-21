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
 *	@file		OpenFilesDialog.cpp
 *	@brief		OpenFilesDialog クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *  @date		2008-01-06 Created
 */

#include "Prefix.h"
#include "OpenFilesDialog.h"
#include <TextControl.h>
#include <Button.h>
#include <String.h>
#include <Path.h>
#include "StringIDs.h"
#include "CommandIDs.h"
#include "LocationInput.h"
#include "PonpokoDiffApp.h"
#include "TextDiffWnd.h"

static const char NAME_BASE_VIEW[] = "BaseView";
static const char NAME_LEFT_TEXT_CONTROL[] = "LeftTextControl";
static const char NAME_LEFT_BROWSE_BUTTON[] = "LeftBrowseButton";
static const char NAME_RIGHT_TEXT_CONTROL[] = "RightTextControl";
static const char NAME_RIGHT_BROWSE_BUTTON[] = "RightBrowseButton";
static const char NAME_DIFF_THEM_BUTTON[] = "DiffThemButton";
static const char NAME_CANCEL_BUTTON[] = "CancelButton";

/**
 *	@brief	コンストラクタ
 *	@param[in]	topLeft	ダイアログの左上位置
 */
OpenFilesDialog::OpenFilesDialog(BPoint topLeft)
	: BWindow(BRect(topLeft.x, topLeft.y, topLeft.x + 460, topLeft.y + 120),
				RT(IDS_TITLE_OPEN_FILES),
				B_TITLED_WINDOW_LOOK,
				B_NORMAL_WINDOW_FEEL,
				B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_NOT_MINIMIZABLE)
{
	int index;
	for (index = 0; index < FileMAX; index++)
	{
		filePanels[index] = NULL;
	}
}

/**
 *	@brief	デストラクタ
 */
OpenFilesDialog::~OpenFilesDialog()
{
	int index;
	for (index = 0; index < FileMAX; index++)
	{
		if (NULL != filePanels[index])
		{
			delete filePanels[index];
		}
	}
}

/**
 *	@brief	初期化を行います。
 */
void OpenFilesDialog::Initialize()
{
	BRect bounds = Bounds();
	
	// 下地となるビュー
	BView* baseView = new BView(bounds, NAME_BASE_VIEW, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	AddChild(baseView);
	baseView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	// TODO: ダイアログのレイアウトについてはもう少し検討が必要
	LocationInput* leftTextControl = new LocationInput(BRect(12, 14, 362, 34), NAME_LEFT_TEXT_CONTROL,
			RT(IDS_LABEL_LEFT_FILE));
	leftTextControl->SetDivider(72);
	baseView->AddChild(leftTextControl);
	
	BButton* leftBrowseButton = new BButton(BRect(368, 12, 448, 36), NAME_LEFT_BROWSE_BUTTON, RT(IDS_LABEL_BROWSE_LEFT), new BMessage(ID_OFD_BROWSE_LEFT));
	baseView->AddChild(leftBrowseButton);
	
	LocationInput* rightTextControl = new LocationInput(BRect(12, 50, 362, 70), NAME_RIGHT_TEXT_CONTROL,
			RT(IDS_LABEL_RIGHT_FILE));
	rightTextControl->SetDivider(72);
	baseView->AddChild(rightTextControl);

	BButton* rightBrowseButton = new BButton(BRect(368, 48, 448, 72), NAME_RIGHT_BROWSE_BUTTON, RT(IDS_LABEL_BROWSE_RIGHT), new BMessage(ID_OFD_BROWSE_RIGHT));
	baseView->AddChild(rightBrowseButton);
	
	BButton* diffThemButton = new BButton(BRect(368, 84, 448, 108), NAME_DIFF_THEM_BUTTON, RT(IDS_LABEL_DIFF_THEM), new BMessage(ID_OFD_DIFF_THEM));
	baseView->AddChild(diffThemButton);
	
	BButton* cancelButton = new BButton(BRect(282, 84, 362, 108), NAME_CANCEL_BUTTON, RT(IDS_LABEL_CANCEL), new BMessage(ID_CANCEL));
	baseView->AddChild(cancelButton);
	
	Show();
}

/**
 *	@brief	メッセージを受信したら呼び出されます。
 *	@param[in]	message	受信したメッセージ
 */
void OpenFilesDialog::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
	case ID_CANCEL:
		PostMessage(B_QUIT_REQUESTED);
		break;

	case ID_OFD_BROWSE_LEFT:
		doBrowseFile(LeftFile);
		break;
	case ID_OFD_BROWSE_RIGHT:
		doBrowseFile(RightFile);
		break;
	
	case ID_OFD_LEFT_SELECTED:
		doFileSelected(LeftFile, message);
		break;
	case ID_OFD_RIGHT_SELECTED:
		doFileSelected(RightFile, message);
		break;
	
	case ID_OFD_DIFF_THEM:
		doDiffThem();
		break;
		
	default:
		BWindow::MessageReceived(message);
		break;
	}
}

/**
 *	@brief	Browse ボタンが押されたときの処理
 *	@param[in]	fileIndex	どっちのファイルか
 */
void OpenFilesDialog::doBrowseFile(OpenFilesDialog::FileIndex fileIndex)
{
	if (fileIndex < 0 || fileIndex >= FileMAX)
	{
		// 無効な範囲
		return;
	}

	if (NULL == filePanels[fileIndex])
	{
		BMessage* message = NULL;
		BString title;
		title += RT(IDS_APPNAME);
		title += ": ";
		switch (fileIndex)
		{
		case LeftFile:
			message = new BMessage(ID_OFD_LEFT_SELECTED);
			title += RT(IDS_TITLE_SELECT_LEFT_FILE);
			break;
		case RightFile:
			message = new BMessage(ID_OFD_RIGHT_SELECTED);
			title += RT(IDS_TITLE_SELECT_RIGHT_FILE);
			break;
		default:
			message = NULL;	// ここには来ない
			break;
		}
		filePanels[fileIndex] = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, NULL, false, true);
		filePanels[fileIndex]->SetTarget(BMessenger(this));
		filePanels[fileIndex]->SetMessage(message);
		delete message;
		filePanels[fileIndex]->Window()->SetTitle(title.String());
	}
	if (!filePanels[fileIndex]->IsShowing())
	{
		filePanels[fileIndex]->Show();
	}
	else
	{
		filePanels[fileIndex]->Window()->Activate();
	}
}

/**
 *	@brief	ファイルが選択されたときの処理
 *	@param[in]	fileIndex	どっちのファイルか
 *	@param[in]	message	選択されたパラメータを含むメッセージ
 */
void OpenFilesDialog::doFileSelected(OpenFilesDialog::FileIndex fileIndex, BMessage* message)
{
	entry_ref ref;
	if (B_OK != message->FindRef("refs", &ref))
	{
		return;
	}
	
	BPath path(&ref);
	const char* viewName;
	switch (fileIndex)
	{
	case LeftFile:
		viewName = NAME_LEFT_TEXT_CONTROL;
		break;
	case RightFile:
		viewName = NAME_RIGHT_TEXT_CONTROL;
		break;
	default:
		viewName = NULL;	// ここには来ない
		break;
	}
	
	BTextControl* textControl = dynamic_cast<BTextControl*>(FindView(viewName));
	if (NULL != textControl)
	{
		textControl->SetText(path.Path());
	}
}

/**
 *	@brief	Diff Them ボタンの処理
 */
void OpenFilesDialog::doDiffThem()
{
	const char* text = NULL;
	BTextControl* textControl;
	
	// 左側
	text = NULL;
	textControl = dynamic_cast<BTextControl*>(FindView(NAME_LEFT_TEXT_CONTROL));
	if (NULL != textControl)
	{
		text = textControl->Text();
	}
	if (NULL == text || '\0' == text[0])
	{
		// 左側ファイルが指定されていません
		// TODO:
		return;
	}
	BPath leftPath(text);
	
	// 右側
	text = NULL;
	textControl = dynamic_cast<BTextControl*>(FindView(NAME_RIGHT_TEXT_CONTROL));
	if (NULL != textControl)
	{
		text = textControl->Text();
	}
	if (NULL == text || '\0' == text[0])
	{
		// 左側ファイルが指定されていません
		// TODO:
		return;
	}
	BPath rightPath(text);
	
	PonpokoDiffApp* app = static_cast<PonpokoDiffApp*>(be_app);
	TextDiffWnd* newDiffWnd = app->NewTextDiffWnd();
	newDiffWnd->ExecuteDiff(leftPath, rightPath, NULL, NULL);
	PostMessage(B_QUIT_REQUESTED);
}

/**
 *	@brief	ウィンドウが閉じるときに呼び出されます。
 */
void OpenFilesDialog::Quit()
{
	// アプリケーションにウィンドウが閉じることを伝える
	PonpokoDiffApp* app = static_cast<PonpokoDiffApp*>(be_app);
	app->OpenFilesDialogClosed();
	
	BWindow::Quit();	
}
