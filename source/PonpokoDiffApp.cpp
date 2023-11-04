/*
 * PonpokoDiff
 *
 * Copyright (c) 2008-2009 PonpokoDiff Project Contributors
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
 *	@file		PonpokoDiffApp.cpp
 *	@brief		PonpokoDiffApp クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-16 Created
 */

#include "PonpokoDiffApp.h"

#include <AboutWindow.h>
#include <Alert.h>
#include <AppFileInfo.h>
#include <Autolock.h>
#include <Catalog.h>
#include <File.h>
#include <Path.h>
#include <Roster.h>
#include <Screen.h>
#include <String.h>

#include "CommandIDs.h"
#include "OpenFilesDialog.h"
#include "Prefix.h"
#include "StringIDs.h"
#include "TextDiffWnd.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


const char* kAppSignature = "application/x-vnd.Hironytic-PonpokoDiff";
static const char OPTION_LABEL[] = "L";		///< オプション「次のパラメータはラベル」

/**
 *	@brief	コンストラクタ
 */
PonpokoDiffApp::PonpokoDiffApp()
	: BApplication("application/x-vnd.Hironytic-PonpokoDiff")
{
	textDiffWndCount = 0;
	openFilesDialog = NULL;
}

/**
 *	@brief	デストラクタ
 */
PonpokoDiffApp::~PonpokoDiffApp()
{
}

/**
 *	@brief アプリケーションが実行される直前に呼び出されます。
 */
void
PonpokoDiffApp::ReadyToRun()
{
	if (0 == textDiffWndCount)
		doOpenFileDialog();
}

/**
 *	@brief	about ダイアログを表示するよう要求されたときに呼ばれます。
 */
void
PonpokoDiffApp::AboutRequested()
{
	BAboutWindow* aboutwindow
		= new BAboutWindow(B_TRANSLATE_SYSTEM_NAME("PonpokoDiff"), kAppSignature);

	const char* authors[] = {
		"2015-2023 HaikuArchives Team",
		NULL
	};

	aboutwindow->AddCopyright(2007, "ICHIMIYA Hironori (Hiron)");
	aboutwindow->AddAuthors(authors);
	aboutwindow->AddDescription(B_TRANSLATE("A graphical file comparison utility."));
	aboutwindow->Show();
}


void
PonpokoDiffApp::makeVersionString(BString& versionString)
{
	versionString = "Version ";
	
	app_info appInfo;
	BFile file;
	BAppFileInfo appFileInfo;
	
	be_app->GetAppInfo(&appInfo);
	file.SetTo(&appInfo.ref, B_READ_ONLY);
	appFileInfo.SetTo(&file);
	
	version_info verInfo;
	if (B_OK == appFileInfo.GetVersionInfo(&verInfo, B_APP_VERSION_KIND))
		versionString += verInfo.short_info;
}

/**
 *	@brief	プログラムの引数を処理するために呼ばれます。
 *	@param[in]	argc	引数の個数
 *	@param[in]	argv	引数
 */
void
PonpokoDiffApp::ArgvReceived(int32 argc, char** argv)
{
	BMessage refsMsg;
	bool isLabel = false;
	for (int32 ix = 1; ix < argc; ix++) {
		if ('-' == argv[ix][0])		// オプション
			isLabel = strcmp(&argv[ix][1], OPTION_LABEL) == 0;	// ラベル指定
		else if (isLabel == true) {	// 手前にラベル指定オプションがあった場合
			refsMsg.AddString("labels", argv[ix]);
			isLabel = false;
		} else {						// ファイル名
			entry_ref ref;
			if (BEntry(argv[ix]).GetRef(&ref) == B_OK)
				refsMsg.AddRef("refs", &ref);
		}
	}
	if (refsMsg.IsEmpty() == false)
		RefsReceived(&refsMsg);
}


void
PonpokoDiffApp::RefsReceived(BMessage* message)
{
	const char* lastLabel;
	BPath lastPath;
	entry_ref ref;
	for (int i = 0; message->FindRef("refs", i, &ref) == B_OK; i++) {
		BPath path = BPath(&ref);
		const char* label = message->GetString("labels", i, NULL);

		if (lastPath.InitCheck() == B_OK && path.InitCheck() == B_OK) {
			TextDiffWnd* wnd = NewTextDiffWnd();
			wnd->ExecuteDiff(lastPath, path, lastLabel, label);
			lastPath.Unset();
		} else {
			lastPath = path;
			lastLabel = label;
		}
	}

	// If there's a stray ref (odd amount), then populate an open dialog
	if (lastPath.InitCheck() == B_OK) {
		entry_ref lastRef;
		BEntry(lastPath.Path()).GetRef(&lastRef);
		BMessage msg(ID_OFD_LEFT_SELECTED);
		msg.AddRef("refs", &lastRef);
		doOpenFileDialog();
		openFilesDialog->MessageReceived(&msg);
	}
}


/**
 *	@brief	新しく TextDiffWnd を生成して表示します。
 *	@return	生成した TextDiffWnd のポインタ。
 */
TextDiffWnd*
PonpokoDiffApp::NewTextDiffWnd()
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		BRect frameRect;
		makeNewTextDiffWndRect(frameRect);
		TextDiffWnd* newWindow = new TextDiffWnd(frameRect, RT(IDS_APPNAME));
		textDiffWndCount++;
		newWindow->Initialize();
		return newWindow;
	}
	
	return NULL;
}

/**
 *	@brief	新しく生成する TextDiffWnd のフレーム矩形を求めます。
 *	@param[out]	frameRect	ここにフレーム矩形が設定されます。
 */
void
PonpokoDiffApp::makeNewTextDiffWndRect(BRect& frameRect)
{
	BRect screenFrame;
	{
		BScreen screen;
		screenFrame = screen.Frame();
	}
	
	float screenWidth = screenFrame.Width() + 1;
	float screenHeight = screenFrame.Height() + 1;
	
	frameRect.left = floor(screenWidth / 8);
	frameRect.top = floor(screenHeight / 8);
	frameRect.right = frameRect.left + floor(screenWidth / 2) - 1;
	frameRect.bottom = frameRect.top + floor(screenHeight / 2) - 1;
}

/**
 *	@brief	TextDiffWnd の Quit が呼ばれたときに呼び出されます。
 *	@param[in]	wnd	Quit が呼ばれた TextDiffWnd
 */
void
PonpokoDiffApp::TextDiffWndQuit(TextDiffWnd* /* wnd */)
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		textDiffWndCount--;
		if (textDiffWndCount <= 0)
			PostMessage(B_QUIT_REQUESTED);
	}
}

/**
 *	@brief	ファイルを開くダイアログが閉じたときに呼び出されます。
 */
void
PonpokoDiffApp::OpenFilesDialogClosed()
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		openFilesDialog = NULL;
		if (textDiffWndCount <= 0)
			PostMessage(B_QUIT_REQUESTED);
	}	
}

/**
 *	@brief	メッセージを受信したら呼び出されます。
 *	@param[in]	message	受信したメッセージ
 */
void
PonpokoDiffApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case ID_FILE_OPEN:
			doOpenFileDialog();
			break;
	
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

/**
 *	@brief	ファイルを開くダイアログを表示します。
 */
void
PonpokoDiffApp::doOpenFileDialog()
{
	if (NULL != openFilesDialog) {
		BAutolock locker(openFilesDialog);
		if (locker.IsLocked()) {
			// すでに存在するならアクティブにするだけ
			if (openFilesDialog->IsMinimized())
				openFilesDialog->Minimize(false);
			if (!openFilesDialog->IsActive())
				openFilesDialog->Activate(true);
			return;
		}
	}
	
	openFilesDialog = new OpenFilesDialog(BPoint(100, 100));
	openFilesDialog->Initialize();
}

/**
 *	@brief	プログラムのエントリポイント
 */
int
main(int /*argc*/, char** /*argv*/)
{
	PonpokoDiffApp app;
	app.Run();
	return 0;
}
