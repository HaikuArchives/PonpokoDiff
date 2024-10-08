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
#include <PathFinder.h>
#include <Roster.h>
#include <Screen.h>
#include <String.h>

#include <cstdio>

#include "CommandIDs.h"
#include "OpenFilesDialog.h"
#include "DiffWindow.h"
#include "TextFileFilter.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


const char* kAppSignature = "application/x-vnd.Hironytic-PonpokoDiff";


App::App()
	:
	BApplication(kAppSignature)
{
	fWindowCount = 0;
	fOpenFilesPanel = NULL;
	fSettings = NULL;

	_LoadSettings();
}


App::~App()
{
}


void
App::ReadyToRun()
{
	if (fWindowCount == 0)
		_OpenFilesPanel(fSettings);
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

	// Approximate nice looking window size
	font_height fh;
	BFont font;
	font.GetHeight(&fh);
	float height = static_cast<float>(ceil(fh.ascent + fh.descent + fh.leading));
	float width = font.StringWidth("Adrien Destugues (PulkoMandy)");

	aboutwindow->ResizeTo(width * 2, height * 21);

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
		TextFileFilter filter;
		if (!filter.IsValid(&ref, &entry)) {
			printf(B_TRANSLATE("The file '%s' isn't a text file!\n"), path.Path());
			continue;
		}
		if (lastPath.InitCheck() == B_OK && path.InitCheck() == B_OK) {
			DiffWindow* window = NewDiffWindow();
			window->ExecuteDiff(lastPath, path);
			lastPath.Unset();
		} else
			lastPath = path;
	}

	// If there's a stray ref (odd number), then populate an open dialog
	if (lastPath.InitCheck() == B_OK) {
		entry_ref lastRef;
		BEntry(lastPath.Path()).GetRef(&lastRef);
		BMessage msg(MSG_OFD_LEFT_SELECTED);
		msg.AddRef("refs", &lastRef);
		_OpenFilesPanel(fSettings);
		fOpenFilesPanel->MessageReceived(&msg);
	}
}


DiffWindow*
App::NewDiffWindow()
{
	BAutolock locker(this);
	if (locker.IsLocked()) {
		DiffWindow* newWindow = new DiffWindow(fSettings, fWindowCount);
		fWindowCount++;
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
			_OpenFilesPanel(message);

			BMessenger window;
			if (message->FindMessenger("killme", &window) == B_OK)
				window.SendMessage(new BMessage(B_QUIT_REQUESTED));

		} break;

		case MSG_HELP:
		{
			_HelpWindow();
		} break;

		default:
			BApplication::MessageReceived(message);
			break;
	}
}


void
App::_HelpWindow()
{
	BAboutWindow* helpWindow
		= new BAboutWindow(B_TRANSLATE("Help"), kAppSignature);
	helpWindow->SetVersion(B_TRANSLATE(
		"Only a few notes on the non-obvious" B_UTF8_ELLIPSIS));

	helpWindow->AddDescription(B_TRANSLATE(
		"Yellow indicates changed lines.\n"
		"Red indicates removed lines.\n"
		"Green indicates added lines.\n\n"
		"Double-click to open the left/right file with its preferred application.\n"
		"Hold CTRL while double-clicking to show the left/right file's location.\n\n"
		"You can drag'n'drop files directly on the left/right side of the window."));

	// Approximate nice looking window size
	font_height fh;
	BFont font;
	font.GetHeight(&fh);
	float height = static_cast<float>(ceil(fh.ascent + fh.descent + fh.leading));
	float width = font.StringWidth(
		"You can drag'n'drop files directly on the left/right side of the window.");

	helpWindow->ResizeTo(width, height * 21);
	helpWindow->Show();
}


void
App::_LoadSettings()
{
	fSettings = new BMessage();

	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	BString settingsFile(B_TRANSLATE_SYSTEM_NAME("PonpokoDiff"));
	settingsFile << "_settings";
	path.Append(settingsFile.String());
	BFile file(path.Path(), B_READ_ONLY);

	if (fSettings->Unflatten(&file) != B_OK)
		fSettings->AddRect("window_frame", BRect(100, 75, 700, 575));
}


void
App::_OpenFilesPanel(BMessage* message)
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
	fOpenFilesPanel = new OpenFilesDialog(message);
}


int
main(int /*argc*/, char** /*argv*/)
{
	App app;
	app.Run();
	return 0;
}
