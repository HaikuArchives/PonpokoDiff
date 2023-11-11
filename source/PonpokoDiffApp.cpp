/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Jaidyn Ann
 *		Humdinger
 *
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

#include <cstdio>

#include "CommandIDs.h"
#include "OpenFilesDialog.h"
#include "TextDiffWnd.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


const char* kAppSignature = "application/x-vnd.Hironytic-PonpokoDiff";


PonpokoDiffApp::PonpokoDiffApp()
	:
	BApplication("application/x-vnd.Hironytic-PonpokoDiff")
{
	textDiffWndCount = 0;
	openFilesDialog = NULL;
}


PonpokoDiffApp::~PonpokoDiffApp()
{
}


void
PonpokoDiffApp::ReadyToRun()
{
	if (0 == textDiffWndCount)
		doOpenFileDialog();
}


void
PonpokoDiffApp::AboutRequested()
{
	BAboutWindow* aboutwindow
		= new BAboutWindow(B_TRANSLATE_SYSTEM_NAME("PonpokoDiff"), kAppSignature);

	const char* authors[] = {
		"ICHIMIYA Hironori (Hiron)",
		"Adrien Destugues (PulkoMandy)",
		"Humdinger",
		"Mark Hellegers",
		NULL
	};

	aboutwindow->AddCopyright(2007, "ICHIMIYA Hironori (Hiron)");
	aboutwindow->AddAuthors(authors);
	aboutwindow->AddDescription(B_TRANSLATE("A graphical file comparison utility."));
	aboutwindow->Show();
}


void
PonpokoDiffApp::ArgvReceived(int32 argc, char** argv)
{
	BMessage refsMsg;
	bool isLabel = false;
	for (int32 ix = 1; ix < argc; ix++) {
		entry_ref ref;
		if (BEntry(argv[ix]).GetRef(&ref) == B_OK)
			refsMsg.AddRef("refs", &ref);
	}

	if (refsMsg.IsEmpty() == false)
		RefsReceived(&refsMsg);
}


void
PonpokoDiffApp::RefsReceived(BMessage* message)
{
	BPath lastPath;
	entry_ref ref;
	for (int i = 0; message->FindRef("refs", i, &ref) == B_OK; i++) {
		BPath path = BPath(&ref);
		BEntry entry(&ref);
		if (!entry.Exists()) {
			printf(B_TRANSLATE("The file '%s' does not exist!\n"), path.Path());
			continue;
		}
		if (lastPath.InitCheck() == B_OK && path.InitCheck() == B_OK) {
			TextDiffWnd* wnd = NewTextDiffWnd();
			wnd->ExecuteDiff(lastPath, path);
			lastPath.Unset();
		} else
			lastPath = path;
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


TextDiffWnd*
PonpokoDiffApp::NewTextDiffWnd()
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		BRect frameRect;
		makeNewTextDiffWndRect(frameRect);
		TextDiffWnd* newWindow = new TextDiffWnd(frameRect, B_TRANSLATE_SYSTEM_NAME("PonpokoDiff"));
		textDiffWndCount++;
		newWindow->Initialize();
		return newWindow;
	}

	return NULL;
}


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


void
PonpokoDiffApp::TextDiffWndQuit(TextDiffWnd* /* wnd */)
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		textDiffWndCount--;
		if (textDiffWndCount <= 0 && openFilesDialog == NULL)
			PostMessage(B_QUIT_REQUESTED);
	}
}


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


void
PonpokoDiffApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case ID_FILE_OPEN:
		{
			doOpenFileDialog();

			BMessenger window;
			if (message->FindMessenger("killme", &window) == B_OK)
				window.SendMessage(new BMessage(B_QUIT_REQUESTED));

		} break;

		default:
			BApplication::MessageReceived(message);
			break;
	}
}


void
PonpokoDiffApp::doOpenFileDialog()
{
	if (openFilesDialog != NULL) {
		BAutolock locker(openFilesDialog);
		if (locker.IsLocked()) {
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


int
main(int /*argc*/, char** /*argv*/)
{
	PonpokoDiffApp app;
	app.Run();
	return 0;
}
