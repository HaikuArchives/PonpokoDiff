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

#include "App.h"

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
#include "DiffWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


const char* kAppSignature = "application/x-vnd.Hironytic-PonpokoDiff";


App::App()
	:
	BApplication("application/x-vnd.Hironytic-PonpokoDiff")
{
	fWindowCount = 0;
	fOpenFilesPanel = NULL;
}


App::~App()
{
}


void
App::ReadyToRun()
{
	if (0 == fWindowCount)
		OpenFilesPanel();
}


void
App::AboutRequested()
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
App::ArgvReceived(int32 argc, char** argv)
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
App::RefsReceived(BMessage* message)
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
			DiffWindow* window = NewDiffWindow();
			window->ExecuteDiff(lastPath, path);
			lastPath.Unset();
		} else
			lastPath = path;
	}

	// If there's a stray ref (odd amount), then populate an open dialog
	if (lastPath.InitCheck() == B_OK) {
		entry_ref lastRef;
		BEntry(lastPath.Path()).GetRef(&lastRef);
		BMessage msg(MSG_OFD_LEFT_SELECTED);
		msg.AddRef("refs", &lastRef);
		OpenFilesPanel();
		fOpenFilesPanel->MessageReceived(&msg);
	}
}


DiffWindow*
App::NewDiffWindow()
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		DiffWindow* newWindow = new DiffWindow();
		fWindowCount++;
		newWindow->Initialize();
		return newWindow;
	}

	return NULL;
}


void
App::DiffWindowQuit(DiffWindow* /* window */)
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		fWindowCount--;
		if (fWindowCount <= 0 && fOpenFilesPanel == NULL)
			PostMessage(B_QUIT_REQUESTED);
	}
}


void
App::OpenFilesPanelClosed()
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		fOpenFilesPanel = NULL;
		if (fWindowCount <= 0)
			PostMessage(B_QUIT_REQUESTED);
	}
}


void
App::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_FILE_OPEN:
		{
			OpenFilesPanel();

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
App::OpenFilesPanel()
{
	if (fOpenFilesPanel != NULL) {
		BAutolock locker(fOpenFilesPanel);
		if (locker.IsLocked()) {
			if (fOpenFilesPanel->IsMinimized())
				fOpenFilesPanel->Minimize(false);
			if (!fOpenFilesPanel->IsActive())
				fOpenFilesPanel->Activate(true);
			return;
		}
	}

	fOpenFilesPanel = new OpenFilesDialog(BPoint(100, 100));
	fOpenFilesPanel->Initialize();
}


int
main(int /*argc*/, char** /*argv*/)
{
	App app;
	app.Run();
	return 0;
}
