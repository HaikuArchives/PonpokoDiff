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
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include "App.h"
#include "CommandIDs.h"
#include "LocationInput.h"
#include "DiffWindow.h"
#include "TextFileFilter.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "OpenFilesDialog"


OpenFilesDialog::OpenFilesDialog(BPoint topLeft)
	:
	BWindow(BRect(topLeft.x, topLeft.y, topLeft.x + 480, topLeft.y + 220),
		B_TRANSLATE("PonpokoDiff: Select files"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
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
OpenFilesDialog::Initialize()
{

	BStringView* leftLabel = new BStringView("leftlabel", B_TRANSLATE("Left file:"));
	LocationInput* leftTextControl = new LocationInput("LeftTextControl", NULL);
	BButton* leftBrowseButton = new BButton(
		"LeftBrowse", B_TRANSLATE("Browse" B_UTF8_ELLIPSIS), new BMessage(MSG_OFD_BROWSE_LEFT));

	BStringView* rightLabel = new BStringView("rightLabel", B_TRANSLATE("Right file:"));
	LocationInput* rightTextControl = new LocationInput("RightTextControl", NULL);
	BButton* rightBrowseButton = new BButton(
		"RightBrowse", B_TRANSLATE("Browse" B_UTF8_ELLIPSIS), new BMessage(MSG_OFD_BROWSE_RIGHT));

	BButton* diffThemButton
		= new BButton("DiffButton", B_TRANSLATE("Diff"), new BMessage(MSG_OFD_DIFF_THEM));
	diffThemButton->MakeDefault(true);

	BButton* cancelButton
		= new BButton("CancelButton", B_TRANSLATE("Cancel"), new BMessage(MSG_CANCEL));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGrid(B_USE_SMALL_SPACING)
			.Add(leftLabel, 0, 0)
			.Add(leftTextControl, 1, 0)
			.Add(leftBrowseButton, 2, 0)
			.Add(rightLabel, 0, 1)
			.Add(rightTextControl, 1, 1)
			.Add(rightBrowseButton, 2, 1)
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(cancelButton)
			.Add(diffThemButton)
			.AddGlue()
		.End();

	Show();
}


void
OpenFilesDialog::MessageReceived(BMessage* message)
{
	switch (message->what) {
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
		fFilePanels[fileIndex] = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL,
			new TextFileFilter(), false, true);
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
OpenFilesDialog::_FileSelected(OpenFilesDialog::FileIndex fileIndex, BMessage* message)
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
		textControl->MarkAsInvalid(false); // in case it was invalid from a file drop before
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
