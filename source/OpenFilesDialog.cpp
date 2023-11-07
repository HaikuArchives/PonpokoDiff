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
#include <NodeInfo.h>
#include <Path.h>
#include <SeparatorView.h>
#include <String.h>
#include <TextControl.h>
#include <Volume.h>

#include "CommandIDs.h"
#include "LocationInput.h"
#include "PonpokoDiffApp.h"
#include "TextDiffWnd.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "OpenFilesDialog"


class TextFilter : public BRefFilter {
public:
	bool	Filter(const entry_ref* entryRef, BNode* node,
				struct stat_beos* stat, const char* fileType);
};


bool
TextFilter::Filter(const entry_ref* ref, BNode* node, struct stat_beos* stat,
	const char* fileType)
{
	BEntry entry(ref, true); // traverse links

	// allow folders and links of folders
	if (entry.IsDirectory())
		return true;

	// allow text and linked text files
	char mimeType[B_MIME_TYPE_LENGTH];
	BNode traversedNode(&entry); // create a new node from the link-traversed BEntry
	BNodeInfo(&traversedNode).GetType(mimeType);
	if (strncmp("text/", mimeType, 5) == 0)
		return true;

	// allow all, if volume doesn't know MIME
	BVolume volume;
	volume.SetTo((ref->device));
	if (volume.KnowsMime() == false)
		return true;

	return false;
}


OpenFilesDialog::OpenFilesDialog(BPoint topLeft)
	:
	BWindow(BRect(topLeft.x, topLeft.y, topLeft.x + 480, topLeft.y + 220),
		B_TRANSLATE("PonpokoDiff: Open files"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	int index;
	for (index = 0; index < FileMAX; index++)
		filePanels[index] = NULL;
}


OpenFilesDialog::~OpenFilesDialog()
{
	int index;
	for (index = 0; index < FileMAX; index++) {
		if (NULL != filePanels[index])
			delete filePanels[index];
	}
}


void
OpenFilesDialog::Initialize()
{
	// TODO: ダイアログのレイアウトについてはもう少し検討が必要
	LocationInput* leftTextControl
		= new LocationInput("LeftTextControl", B_TRANSLATE("Left file:"));

	BButton* leftBrowseButton = new BButton(
		"LeftBrowse", B_TRANSLATE("Browse" B_UTF8_ELLIPSIS), new BMessage(ID_OFD_BROWSE_LEFT));

	LocationInput* rightTextControl
		= new LocationInput("RightTextControl", B_TRANSLATE("Right file:"));

	BButton* rightBrowseButton = new BButton(
		"RightBrowse", B_TRANSLATE("Browse" B_UTF8_ELLIPSIS), new BMessage(ID_OFD_BROWSE_RIGHT));

	BButton* diffThemButton
		= new BButton("DiffButton", B_TRANSLATE("Diff"), new BMessage(ID_OFD_DIFF_THEM));
	diffThemButton->MakeDefault(true);

	BButton* cancelButton
		= new BButton("CancelButton", B_TRANSLATE("Cancel"), new BMessage(ID_CANCEL));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGrid(B_USE_SMALL_SPACING)
			.Add(leftTextControl->CreateLabelLayoutItem(), 0, 0)
			.Add(leftTextControl->CreateTextViewLayoutItem(), 1, 0)
			.Add(leftBrowseButton, 2, 0)
			.Add(rightTextControl->CreateLabelLayoutItem(), 0, 1)
			.Add(rightTextControl->CreateTextViewLayoutItem(), 1, 1)
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
		{
			if (Lock()) {
				doFileSelected(LeftFile, message);
				Unlock();
			}
		} break;

		case ID_OFD_RIGHT_SELECTED:
		{
			if (Lock()) {
				doFileSelected(RightFile, message);
				Unlock();
			}
		} break;

		case ID_OFD_DIFF_THEM:
			doDiffThem();
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
OpenFilesDialog::doBrowseFile(OpenFilesDialog::FileIndex fileIndex)
{
	if (fileIndex < 0 || fileIndex >= FileMAX) {
		// 無効な範囲
		return;
	}

	if (NULL == filePanels[fileIndex]) {
		BMessage* message = NULL;
		BString title;
		title += B_TRANSLATE_SYSTEM_NAME("PonpokoDiff");
		title += ": ";
		switch (fileIndex) {
			case LeftFile:
			{
				message = new BMessage(ID_OFD_LEFT_SELECTED);
				title += B_TRANSLATE("Select left file");
			} break;

			case RightFile:
			{
				message = new BMessage(ID_OFD_RIGHT_SELECTED);
				title += B_TRANSLATE("Select right file");
			} break;

			default:
				message = NULL;
				break;
		}
		filePanels[fileIndex] = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL,
			new TextFilter(), false, true);
		filePanels[fileIndex]->SetTarget(BMessenger(this));
		filePanels[fileIndex]->SetMessage(message);
		delete message;
		filePanels[fileIndex]->Window()->SetTitle(title.String());
	}
	if (!filePanels[fileIndex]->IsShowing())
		filePanels[fileIndex]->Show();
	else
		filePanels[fileIndex]->Window()->Activate();
}


void
OpenFilesDialog::doFileSelected(OpenFilesDialog::FileIndex fileIndex, BMessage* message)
{
	entry_ref ref;
	if (B_OK != message->FindRef("refs", &ref))
		return;

	BPath path(&ref);
	const char* viewName;
	switch (fileIndex) {
		case LeftFile:
			viewName = "LeftTextControl";
			break;

		case RightFile:
			viewName = "RightTextControl";
			break;

		default:
			viewName = NULL;
			break;
	}

	BTextControl* textControl = dynamic_cast<BTextControl*>(FindView(viewName));
	if (NULL != textControl)
		textControl->SetText(path.Path());
}


void
OpenFilesDialog::doDiffThem()
{
	const char* text = NULL;
	BTextControl* textControl;

	text = NULL;
	textControl = dynamic_cast<BTextControl*>(FindView("LeftTextControl"));
	if (NULL != textControl)
		text = textControl->Text();
	if (NULL == text || '\0' == text[0])
		return;

	BPath leftPath(text);

	text = NULL;
	textControl = dynamic_cast<BTextControl*>(FindView("RightTextControl"));
	if (NULL != textControl)
		text = textControl->Text();
	if (NULL == text || '\0' == text[0])
		return;

	BPath rightPath(text);

	PonpokoDiffApp* app = static_cast<PonpokoDiffApp*>(be_app);
	TextDiffWnd* newDiffWnd = app->NewTextDiffWnd();
	newDiffWnd->ExecuteDiff(leftPath, rightPath);
	PostMessage(B_QUIT_REQUESTED);
}


void
OpenFilesDialog::Quit()
{
	PonpokoDiffApp* app = static_cast<PonpokoDiffApp*>(be_app);
	app->OpenFilesDialogClosed();

	BWindow::Quit();
}
