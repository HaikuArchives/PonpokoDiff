/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */

#include "OpenFilesDialog.h"

#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <SeparatorView.h>
#include <Screen.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include "App.h"
#include "CommandIDs.h"
#include "DiffWindow.h"
#include "TextFileFilter.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "OpenFilesDialog"


OpenFilesDialog::OpenFilesDialog(BMessage* settings)
	:
	BWindow(BRect(0, 0, 400, 0),
		B_TRANSLATE("PonpokoDiff: Select files"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	_Initialize();

	BRect frame;
	if (settings->FindRect("window_frame", &frame) == B_OK) {
		BScreen screen(this);
		frame.OffsetBy(32, 32);
		if (frame.Intersects(screen.Frame()))
			MoveTo(frame.LeftTop());
		else
			CenterOnScreen();
	}

	Show();

	int index;
	for (index = 0; index < FileMAX; index++)
		fFilePanels[index] = NULL;
}


OpenFilesDialog::~OpenFilesDialog()
{
	int index;
	for (index = 0; index < FileMAX; index++) {
		if (fFilePanels[index] != NULL)
			delete fFilePanels[index];
	}
}


void
OpenFilesDialog::_Initialize()
{
	BStringView* leftLabel = new BStringView("leftlabel", B_TRANSLATE("Left file:"));
	fLeftLocation = new LocationInput("LeftTextControl", NULL);
	BButton* leftBrowseButton = new BButton("LeftBrowse",
		B_TRANSLATE_COMMENT("Browse" B_UTF8_ELLIPSIS, "Button label"),
		new BMessage(MSG_OFD_BROWSE_LEFT));

	BStringView* rightLabel = new BStringView("rightLabel", B_TRANSLATE("Right file:"));
	fRightLocation = new LocationInput("RightTextControl", NULL);
	BButton* rightBrowseButton = new BButton("RightBrowse",
		B_TRANSLATE_COMMENT("Browse" B_UTF8_ELLIPSIS, "Button label"),
		new BMessage(MSG_OFD_BROWSE_RIGHT));

	BButton* diffButton = new BButton("DiffButton",
		B_TRANSLATE_COMMENT("Diff", "Button label"),
		new BMessage(MSG_OFD_DIFF_THEM));
	diffButton->MakeDefault(true);

	BButton* cancelButton = new BButton("CancelButton",
		B_TRANSLATE_COMMENT("Cancel", "Button label"),
		new BMessage(MSG_CANCEL));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGrid(B_USE_SMALL_SPACING)
			.Add(leftLabel, 0, 0)
			.Add(fLeftLocation, 1, 0)
			.Add(leftBrowseButton, 2, 0)
			.Add(rightLabel, 0, 1)
			.Add(fRightLocation, 1, 1)
			.Add(rightBrowseButton, 2, 1)
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(cancelButton)
			.Add(diffButton)
			.AddGlue()
		.End();
}


void
OpenFilesDialog::MessageReceived(BMessage* message)
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
					// Only allow text files and catkeys
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
				// only invalid if there wasn't a valid file before
				BString text = fLeftLocation->Text();
				if (text == "")
					fLeftLocation->MarkAsInvalid(true);

				text = fRightLocation->Text();
				if (text == "")
					fRightLocation->MarkAsInvalid(true);

				// no valid files
				if (validFileCount == 0)
					break;

				fLeftLocation->SetText(paths[0].Path());
				fLeftLocation->MarkAsInvalid(false);

				if (validFileCount == 2) {
					fRightLocation->SetText(paths[1].Path());
					fRightLocation->MarkAsInvalid(false);
				} else if (text == "")
					fRightLocation->MarkAsInvalid(true);
			}
		} break;

		case MSG_CANCEL:
			PostMessage(B_QUIT_REQUESTED);
			break;

		case MSG_OFD_BROWSE_LEFT:
			_BrowseFile(LEFTFILE);
			break;

		case MSG_OFD_BROWSE_RIGHT:
			_BrowseFile(RIGHTFILE);
			break;

		case MSG_OFD_LEFT_SELECTED:
		{
			if (Lock()) {
				_FileSelected(LEFTFILE, message);
				Unlock();
			}
		} break;

		case MSG_OFD_RIGHT_SELECTED:
		{
			if (Lock()) {
				_FileSelected(RIGHTFILE, message);
				Unlock();
			}
		} break;

		case MSG_OFD_DIFF_THEM:
			_RunDiff();
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
OpenFilesDialog::_BrowseFile(OpenFilesDialog::FileIndex fileIndex)
{
	if (fileIndex < 0 || fileIndex >= FileMAX)
		return;

	if (fFilePanels[fileIndex] == NULL) {
		BMessage* message = NULL;
		BString title;
		title += B_TRANSLATE_SYSTEM_NAME("PonpokoDiff");
		title += ": ";
		switch (fileIndex) {
			case LEFTFILE:
			{
				message = new BMessage(MSG_OFD_LEFT_SELECTED);
				title += B_TRANSLATE("Select left file");
			} break;

			case RIGHTFILE:
			{
				message = new BMessage(MSG_OFD_RIGHT_SELECTED);
				title += B_TRANSLATE("Select right file");
			} break;

			default:
				message = NULL;
				break;
		}
		fFilePanels[fileIndex] = new BFilePanel(B_OPEN_PANEL, NULL, NULL,
			B_FILE_NODE, false, NULL, new TextFileFilter(), false, true);
		fFilePanels[fileIndex]->SetTarget(BMessenger(this));
		fFilePanels[fileIndex]->SetMessage(message);
		delete message;
		fFilePanels[fileIndex]->Window()->SetTitle(title.String());
	}
	if (!fFilePanels[fileIndex]->IsShowing())
		fFilePanels[fileIndex]->Show();
	else
		fFilePanels[fileIndex]->Window()->Activate();
}


void
OpenFilesDialog::_FileSelected(OpenFilesDialog::FileIndex fileIndex,
	BMessage* message)
{
	entry_ref ref;
	if (B_OK != message->FindRef("refs", &ref))
		return;

	BPath path(&ref);
	const char* viewName;
	switch (fileIndex) {
		case LEFTFILE:
			viewName = "LeftTextControl";
			break;

		case RIGHTFILE:
			viewName = "RightTextControl";
			break;

		default:
			viewName = NULL;
			break;
	}

	BTextControl* textControl = dynamic_cast<BTextControl*>(FindView(viewName));
	if (textControl != NULL) {
		textControl->SetText(path.Path());
		// in case it was invalid from a file drop before
		textControl->MarkAsInvalid(false);
	}
}


void
OpenFilesDialog::_RunDiff()
{
	const char* text = NULL;
	BTextControl* textControl;

	text = NULL;
	textControl = dynamic_cast<BTextControl*>(FindView("LeftTextControl"));
	if (textControl != NULL)
		text = textControl->Text();
	if (text == NULL || '\0' == text[0])
		return;

	BPath leftPath(text);

	text = NULL;
	textControl = dynamic_cast<BTextControl*>(FindView("RightTextControl"));
	if (textControl != NULL)
		text = textControl->Text();
	if (text == NULL|| '\0' == text[0])
		return;

	BPath rightPath(text);

	App* app = static_cast<App*>(be_app);
	DiffWindow* window = app->NewDiffWindow();
	window->ExecuteDiff(leftPath, rightPath);
	PostMessage(B_QUIT_REQUESTED);
}


void
OpenFilesDialog::Quit()
{
	App* app = static_cast<App*>(be_app);
	app->OpenFilesPanelClosed();

	BWindow::Quit();
}
