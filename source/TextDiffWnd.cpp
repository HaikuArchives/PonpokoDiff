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
 *	@file		TextDiffWnd.cpp
 *	@brief		TextDiffWnd クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-24 Created
 */

#include "Prefix.h"
#include "TextDiffWnd.h"
#include <Application.h>
#include <Autolock.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <String.h>
#include <Path.h>
#include <Messenger.h>
#include "PonpokoDiffApp.h"
#include "TextDiffView.h"
#include "StringIDs.h"
#include "CommandIDs.h"

static const char NAME_MAIN_MENU[]				= "MainMenu";
static const char NAME_TEXT_DIFF_VIEW[]			= "TextDiffView";

/**
 *	@brief	コンストラクタ
 *	@param[in]	frame		ウィンドウの矩形
 *	@param[in]	name		ウィンドウ名
 *	@param[in]	workspaces	表示ワークスペース
 */
TextDiffWnd::TextDiffWnd(BRect frame, const char* name, uint32 workspaces /* = B_CURRENT_WORKSPACE */)
	:	BWindow(frame, name, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0, workspaces)
{	
}

/**
 *	@brief	デストラクタ
 */
TextDiffWnd::~TextDiffWnd()
{
}

/**
 *	@brief	初期化します。
 *	
 *	オブジェクト生成後に1回だけ呼び出せます。これによりウィンドウが表示されます。
 */
void TextDiffWnd::Initialize()
{
	BRect bounds = Bounds();

	// メインメニュー
	BMenuBar* menuBar = new BMenuBar(Bounds(), NAME_MAIN_MENU);
	createMainMenu(menuBar);
	AddChild(menuBar);

	// diff view
	BRect menuBarFrame = menuBar->Frame();
	BRect diffViewRect = BRect(bounds.left, menuBarFrame.bottom + 1, bounds.right, bounds.bottom);
	TextDiffView* diffView = new TextDiffView(diffViewRect, NAME_TEXT_DIFF_VIEW, B_FOLLOW_ALL_SIDES);
	diffView->Initialize();
	AddChild(diffView);

	Show();
}

/**
 *	@brief	メインメニューを追加します。
 *	@param[in]	menuBar	ここに各メニューが足されます。
 */
void TextDiffWnd::createMainMenu(BMenuBar* menuBar)
{
	BMenuItem* menuItem;

	// File
	BMenu* fileMenu = new BMenu(RT(IDS_MENU_FILE));
	menuBar->AddItem(fileMenu);
	menuItem = new BMenuItem(RT(IDS_MENU_FILE_OPEN), new BMessage(ID_FILE_OPEN), 'O');
	menuItem->SetTarget(be_app_messenger);
	fileMenu->AddItem(menuItem);
	menuItem = new BMenuItem(RT(IDS_MENU_FILE_RELOAD), new BMessage(ID_FILE_RELOAD), 'R');
	menuItem->SetTarget(this);
	fileMenu->AddItem(menuItem);
	fileMenu->AddItem(new BMenuItem(RT(IDS_MENU_FILE_CLOSE), new BMessage(ID_FILE_CLOSE), 'W'));
	fileMenu->AddSeparatorItem();
	menuItem = new BMenuItem(RT(IDS_MENU_FILE_ABOUT), new BMessage(ID_FILE_ABOUT));
	menuItem->SetTarget(be_app_messenger);
	fileMenu->AddItem(menuItem);
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem(RT(IDS_MENU_FILE_QUIT), new BMessage(ID_FILE_QUIT), 'Q'));
}

/**
 *	@brief	Diff を実行します。
 *	@param[in]	pathLeft	左ペインに表示するファイルのパス
 *	@param[in]	pathRight	右ペインに表示するファイルのパス
 */
void TextDiffWnd::ExecuteDiff(const BPath& pathLeft, const BPath& pathRight, const char* labelLeft, const char* labelRight)
{
	fPathLeft = pathLeft;
	fPathRight = pathRight;
	fLabelLeft = labelLeft;
	fLabelRight = labelRight;

	// TODO:
	// ここは最終的にはスクリプティングによるメッセージ送信にしたい。
	BAutolock locker(this);
	if (locker.IsLocked())
	{
		TextDiffView* diffView = dynamic_cast<TextDiffView*>(FindView(NAME_TEXT_DIFF_VIEW));
		if (NULL != diffView)
		{
			diffView->ExecuteDiff(pathLeft, pathRight, labelLeft, labelRight);
		}
	}
	
	// ウィンドウのタイトルにラベルを追加
	BString title;
	if (NULL == labelLeft)
	{
		title += pathLeft.Leaf();
	}
	else
	{
		title += labelLeft;
	}
	title += ", ";
	if (NULL == labelRight)
	{
		title += pathRight.Leaf();
	}
	else
	{
		title += labelRight;
	}
	title += " - ";
	title += RT(IDS_APPNAME);
	SetTitle(title.String());
}

/**
 *	@brief	ウィンドウが閉じるときに呼び出されます。
 */
void TextDiffWnd::Quit()
{
	// アプリケーションに終了を伝える
	PonpokoDiffApp* app = static_cast<PonpokoDiffApp*>(be_app);
	app->TextDiffWndQuit(this);
	
	BWindow::Quit();	
}

/**
 *	@brief	メッセージを受信したら呼び出されます。
 *	@param[in]	message	受信したメッセージ
 */
void TextDiffWnd::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
	case ID_FILE_QUIT:
		doFileQuit();
		break;

	case ID_FILE_RELOAD:
		ExecuteDiff(fPathLeft, fPathRight, fLabelLeft, fLabelRight);
		break;

	default:
		BWindow::MessageReceived(message);
		break;
	}
}

/**
 *	@brief	[File] - [Quit] の処理
 */
void TextDiffWnd::doFileQuit()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
}