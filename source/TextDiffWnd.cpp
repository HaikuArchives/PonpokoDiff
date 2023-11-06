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

#include "TextDiffWnd.h"
#include "CommandIDs.h"
#include "PonpokoDiffApp.h"
#include "TextDiffView.h"

#include <Alert.h>
#include <Application.h>
#include <Autolock.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <NodeInfo.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <String.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TextDiffWindow"


/**
 *	@brief	コンストラクタ
 *	@param[in]	frame		ウィンドウの矩形
 *	@param[in]	name		ウィンドウ名
 *	@param[in]	workspaces	表示ワークスペース
 */
TextDiffWnd::TextDiffWnd(
	BRect frame, const char* name, uint32 workspaces /* = B_CURRENT_WORKSPACE */)
	:
	BWindow(frame, name, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0, workspaces)
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
void
TextDiffWnd::Initialize()
{
	BRect bounds = Bounds();

	// メインメニュー
	BMenuBar* menuBar = new BMenuBar(Bounds(), "MainMenu");
	createMainMenu(menuBar);
	AddChild(menuBar);

	// diff view
	BRect menuBarFrame = menuBar->Frame();
	BRect diffViewRect = BRect(bounds.left, menuBarFrame.bottom + 1, bounds.right, bounds.bottom);
	TextDiffView* diffView
		= new TextDiffView(diffViewRect, "TextDiffView", B_FOLLOW_ALL_SIDES);
	diffView->Initialize();
	AddChild(diffView);

	Show();
}



/**
 *	@brief	Diff を実行します。
 *	@param[in]	pathLeft	左ペインに表示するファイルのパス
 *	@param[in]	pathRight	右ペインに表示するファイルのパス
 */
void
TextDiffWnd::ExecuteDiff(
	const BPath& pathLeft, const BPath& pathRight)
{
	fPathLeft = pathLeft;
	fPathRight = pathRight;

	// TODO:
	// ここは最終的にはスクリプティングによるメッセージ送信にしたい。
	BAutolock locker(this);
	if (locker.IsLocked()) {
		TextDiffView* diffView = dynamic_cast<TextDiffView*>(FindView("TextDiffView"));
		if (NULL != diffView)
			diffView->ExecuteDiff(fPathLeft, fPathRight);
	}

	updateTitle();
	startNodeMonitor();
}

/**
 *	@brief	ウィンドウが閉じるときに呼び出されます。
 */
void
TextDiffWnd::Quit()
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
void
TextDiffWnd::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SIMPLE_DATA:
		{
			if (message->WasDropped()) {
				entry_ref ref;
				if (message->FindRef("refs", &ref) != B_OK)
					break;

				// Only allow text files
				BEntry entry(&ref, true); // traverse links
				BNode node(&entry);
				char mimeType[B_MIME_TYPE_LENGTH];
				BNodeInfo(&node).GetType(mimeType);
				if (!strncmp("text/", mimeType, 5) == 0)
					break;

				// Dropped on the left or right side
				BView* diffView = FindView("TextDiffView");
				BRect rect = ConvertToScreen(diffView->Bounds());
				float viewWidth = (rect.Width() - be_control_look->GetScrollBarWidth(B_VERTICAL))
					/ 2;
				rect.right -= viewWidth;
				BPoint dropPoint = message->DropPoint();
				if (rect.Contains(dropPoint))
					fPathLeft.SetTo(&entry);
				else
					fPathRight.SetTo(&entry);

				ExecuteDiff(fPathLeft, fPathRight);
			}
		}
		case B_NODE_MONITOR:
			handleNodeMonitorEvent(message);
			break;

		case ID_FILE_QUIT:
			doFileQuit();
			break;

		case ID_FILE_RELOAD:
			ExecuteDiff(fPathLeft, fPathRight);
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}

/**
 *	@brief	メインメニューを追加します。
 *	@param[in]	menuBar	ここに各メニューが足されます。
 */
void
TextDiffWnd::createMainMenu(BMenuBar* menuBar)
{
	BMenuItem* menuItem;

	// File
	BMenu* fileMenu = new BMenu(B_TRANSLATE("File"));
	menuBar->AddItem(fileMenu);
	menuItem = new BMenuItem(B_TRANSLATE("Open" B_UTF8_ELLIPSIS), new BMessage(ID_FILE_OPEN), 'O');
	menuItem->SetTarget(be_app_messenger);
	fileMenu->AddItem(menuItem);

	menuItem = new BMenuItem(B_TRANSLATE("Reload"), new BMessage(ID_FILE_RELOAD), 'R');
	menuItem->SetTarget(this);
	fileMenu->AddItem(menuItem);
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Close"), new BMessage(ID_FILE_CLOSE), 'W'));

	fileMenu->AddSeparatorItem();

	menuItem = new BMenuItem(B_TRANSLATE("About PonpokoDiff"), new BMessage(ID_FILE_ABOUT));
	menuItem->SetTarget(be_app_messenger);
	fileMenu->AddItem(menuItem);

	fileMenu->AddSeparatorItem();

	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(ID_FILE_QUIT), 'Q'));
}

/**
 *	@brief	Diff を実行します。
 *	@param[in]	pathLeft	左ペインに表示するファイルのパス
 *	@param[in]	pathRight	右ペインに表示するファイルのパス
 */
void
TextDiffWnd::startNodeMonitor()
{
	BEntry entry(fPathLeft.Path(), true);
	if (entry.InitCheck() == B_OK)
		entry.GetNodeRef(&fLeftNodeRef);

	entry.SetTo(fPathRight.Path(), true);
	if (entry.InitCheck() == B_OK)
		entry.GetNodeRef(&fRightNodeRef);

	watch_node(&fLeftNodeRef, B_WATCH_STAT | B_WATCH_NAME, this);
	watch_node(&fRightNodeRef, B_WATCH_STAT | B_WATCH_NAME, this);
}


void
TextDiffWnd::handleNodeMonitorEvent(BMessage* message)
{
	int32 opcode = 0;
	if (message->FindInt32("opcode", &opcode) != B_OK)
		return;

	switch (opcode) {
		case B_STAT_CHANGED:
		{
			node_ref nref;
			int32 fields;

			if (message->FindInt32("device", &nref.device) != B_OK
				|| message->FindInt64("node", &nref.node) != B_OK
				|| message->FindInt32("fields", &fields) != B_OK)
					break;

			if (((fields & B_STAT_MODIFICATION_TIME)  != 0)
				&& ((fields & B_STAT_ACCESS_TIME)  == 0))
				askToReload(nref);

		} break;

		case B_ENTRY_MOVED:
		{
			message->PrintToStream();

			int32 device = 0;
			int64 dstFolder = 0;
			const char* oldName = NULL;
			const char* newName = NULL;
			if (message->FindInt32("device", &device) != B_OK
				|| message->FindInt64("to directory", &dstFolder) != B_OK
				|| message->FindString("name", &newName) != B_OK
				|| message->FindString("from name", &oldName) != B_OK)
					break;

			entry_ref newRef(device, dstFolder, newName);
			BEntry entry(&newRef);
			if (strcmp(oldName, fPathLeft.Leaf()) == 0) {
				fPathLeft.SetTo(&entry);
				watch_node(&fLeftNodeRef, B_STOP_WATCHING, this); // stop watching old file
				entry.GetNodeRef(&fLeftNodeRef);
				watch_node(&fLeftNodeRef, B_WATCH_STAT | B_WATCH_NAME, this);
			} else if (strcmp(oldName, fPathRight.Leaf()) == 0) {
				fPathRight.SetTo(&entry);
				watch_node(&fRightNodeRef, B_STOP_WATCHING, this); // stop watching old file
				entry.GetNodeRef(&fRightNodeRef);
				watch_node(&fRightNodeRef, B_WATCH_STAT | B_WATCH_NAME, this);
			} else
				break;

			updateTitle();
		} break;
	}
}


void
TextDiffWnd::askToReload(node_ref nref_node)
{
	BString text;
	if (nref_node == fLeftNodeRef) {
		text = B_TRANSLATE(
			"The left file, %filename%, has changed.");
		text.ReplaceFirst("%filename%", fPathLeft.Leaf());
	} else if (nref_node == fRightNodeRef) {
		text = B_TRANSLATE(
			"The right file, %filename%, has changed.");
		text.ReplaceFirst("%filename%", fPathRight.Leaf());
	} else
		return;

	text << "\n" << B_TRANSLATE("Do you want to reload the files and diff them again?");
	BAlert* alert = new BAlert(B_TRANSLATE("A file has changed"), text,
		B_TRANSLATE("Cancel"), B_TRANSLATE("Reload"));
	int32 result = alert->Go();

	switch(result) {
		case 0:
			return;

		case 1:
			ExecuteDiff(fPathLeft, fPathRight);
			break;
	}
}


void
TextDiffWnd::updateTitle()
{
	// ウィンドウのタイトルにラベルを追加
	BString title(B_TRANSLATE_SYSTEM_NAME("PonpokoDiff"));
	title += " : ";
	title += fPathLeft.Leaf();
	title += " ◄ | ► ";
	title += fPathRight.Leaf();

	SetTitle(title.String());

	startNodeMonitor();
}

/**
 *	@brief	[File] - [Quit] の処理
 */
void
TextDiffWnd::doFileQuit()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
}
