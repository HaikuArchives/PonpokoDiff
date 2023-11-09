/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */

#include "TextDiffWnd.h"
#include "CommandIDs.h"
#include "PonpokoDiffApp.h"

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
#include <Roster.h>
#include <String.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TextDiffWindow"


TextDiffWnd::TextDiffWnd(
	BRect frame, const char* name, uint32 workspaces /* = B_CURRENT_WORKSPACE */)
	:
	BWindow(frame, name, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0, workspaces)
{
}


TextDiffWnd::~TextDiffWnd()
{
}


void
TextDiffWnd::Initialize()
{
	BRect bounds = Bounds();

	BMenuBar* menuBar = new BMenuBar(Bounds(), "MainMenu");
	createMainMenu(menuBar);
	AddChild(menuBar);

	// diff view
	BRect menuBarFrame = menuBar->Frame();
	BRect rect = BRect(bounds.left, menuBarFrame.bottom + 1, bounds.right, bounds.bottom);
	fDiffView = new TextDiffView(rect, "TextDiffView", B_FOLLOW_ALL_SIDES);
	fDiffView->Initialize();
	AddChild(fDiffView);

	Show();
}


void
TextDiffWnd::ExecuteDiff(const BPath pathLeft, const BPath pathRight)
{
	fPathLeft = pathLeft;
	fPathRight = pathRight;

	if (fDiffView->LockLooper()) {
		fDiffView->ExecuteDiff(fPathLeft, fPathRight);
		fDiffView->UnlockLooper();
	}

	updateTitle();
	startNodeMonitor();
}


void
TextDiffWnd::Quit()
{
	PonpokoDiffApp* app = static_cast<PonpokoDiffApp*>(be_app);
	app->TextDiffWndQuit(this);

	BWindow::Quit();
}


void
TextDiffWnd::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_NODE_MONITOR:
			handleNodeMonitorEvent(message);
			break;

		case ID_FILE_QUIT:
			doFileQuit();
			break;

		case ID_FILE_LAUNCH:
		{
			int32 pane;
			if (message->FindInt32("pane", &pane) == B_OK) {
				if (pane == 0)
					openFile(fPathLeft);
				else if (pane == 1)
					openFile(fPathRight);
			}
		} break;

		case ID_FILE_DROPPED:
		{
			BString path;
			if (message->FindString("leftpath", &path) == B_OK) {
				watch_node(&fLeftNodeRef, B_STOP_WATCHING, this); // stop watching old file
				fPathLeft.SetTo(path.String());
			} else if (message->FindString("rightpath", &path) == B_OK) {
				watch_node(&fRightNodeRef, B_STOP_WATCHING, this); // stop watching old file
				fPathRight.SetTo(path.String());
			}
		} //intentional fall-through
		case ID_FILE_RELOAD:
		{
			fDiffView->ExecuteDiff(fPathLeft, fPathRight);
			updateTitle();
			startNodeMonitor();
		} break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


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
			startNodeMonitor();
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
			fDiffView->ExecuteDiff(fPathLeft, fPathRight);
			break;
	}
}


void
TextDiffWnd::openFile(BPath path)
{
	entry_ref ref;
	get_ref_for_path(path.Path(), &ref);
	be_roster->Launch(&ref);
}


void
TextDiffWnd::updateTitle()
{
	BString title(B_TRANSLATE_SYSTEM_NAME("PonpokoDiff"));
	title += " : ";
	title += fPathLeft.Leaf();
	title += " ◄ | ► ";
	title += fPathRight.Leaf();

	SetTitle(title.String());
}


void
TextDiffWnd::doFileQuit()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
}
