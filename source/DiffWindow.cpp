/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */

#include "App.h"
#include "CommandIDs.h"
#include "DiffWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Autolock.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
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


DiffWindow::DiffWindow(
	BRect frame, const char* name, uint32 workspaces /* = B_CURRENT_WORKSPACE */)
	:
	BWindow(frame, name, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0, workspaces)
{
}


DiffWindow::~DiffWindow()
{
}


void
DiffWindow::Initialize()
{
	BMenuBar* menuBar = new BMenuBar("MainMenu");
	_CreateMainMenu(menuBar);

	fDiffView = new DiffView("DiffView");
	fDiffView->Initialize();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.AddGroup(B_VERTICAL)
			.SetInsets(0, 0 ,-1, -1)
			.Add(fDiffView)
		.End();

	Show();
}


void
DiffWindow::ExecuteDiff(const BPath pathLeft, const BPath pathRight)
{
	fPathLeft = pathLeft;
	fPathRight = pathRight;

	if (fDiffView->LockLooper()) {
		fDiffView->ExecuteDiff(fPathLeft, fPathRight);
		fDiffView->UnlockLooper();
	}

	_UpdateTitle();
	_StartNodeMonitor();
}


void
DiffWindow::Quit()
{
	App* app = static_cast<App*>(be_app);
	app->DiffWindowQuit(this);

	BWindow::Quit();
}


void
DiffWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_NODE_MONITOR:
			_HandleNodeMonitorEvent(message);
			break;

		case MSG_FILE_QUIT:
			_DoFileQuit();
			break;

		case MSG_FILE_LAUNCH:
		{
			int32 pane;
			if (message->FindInt32("pane", &pane) == B_OK) {
				if (pane == LEFT)
					_OpenFile(fPathLeft);
				else if (pane == RIGHT)
					_OpenFile(fPathRight);
			}
		} break;

		case MSG_OPEN_LOCATION:
		{
			int32 pane;
			if (message->FindInt32("pane", &pane) == B_OK) {
				if (pane == LEFT)
					_OpenLocation(fPathLeft);
				else if (pane == RIGHT)
					_OpenLocation(fPathRight);
			}
		} break;

		case MSG_FILE_DROPPED:
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
		case MSG_FILE_RELOAD:
		{
			// Are the files still there?
			BEntry entryLeft(fPathLeft.Path());
			BEntry entryRight(fPathRight.Path());
			if (!entryLeft.Exists()) {
				_AskFileRemoved(fLeftNodeRef);
				break;
			} else if (!entryRight.Exists()) {
				_AskFileRemoved(fRightNodeRef);
				break;
			}

			fDiffView->ExecuteDiff(fPathLeft, fPathRight);
			_UpdateTitle();
			_StartNodeMonitor();
		} break;

		case MSG_FILE_SWITCH:
		{
			node_ref tempNode = fLeftNodeRef;
			fLeftNodeRef = fRightNodeRef;
			fRightNodeRef = tempNode;

			BPath tempPath = fPathLeft;
			fPathLeft = fPathRight;
			fPathRight = tempPath;

			fDiffView->ExecuteDiff(fPathLeft, fPathRight);
			_UpdateTitle();
		} break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
DiffWindow::_CreateMainMenu(BMenuBar* menuBar)
{
	BMenuItem* menuItem;

	BMenu* fileMenu = new BMenu(B_TRANSLATE("File"));
	menuBar->AddItem(fileMenu);
	menuItem = new BMenuItem(B_TRANSLATE("Choose files" B_UTF8_ELLIPSIS),
		new BMessage(MSG_FILE_OPEN), 'O');
	menuItem->SetTarget(be_app_messenger);
	fileMenu->AddItem(menuItem);

	menuItem = new BMenuItem(B_TRANSLATE("Switch files"), new BMessage(MSG_FILE_SWITCH), B_TAB);
	menuItem->SetTarget(this);
	fileMenu->AddItem(menuItem);

	menuItem = new BMenuItem(B_TRANSLATE("Reload"), new BMessage(MSG_FILE_RELOAD), 'R');
	menuItem->SetTarget(this);
	fileMenu->AddItem(menuItem);

	menuItem = new BMenuItem(B_TRANSLATE("Close"), new BMessage(B_QUIT_REQUESTED), 'W');
	fileMenu->AddItem(menuItem);

	fileMenu->AddSeparatorItem();

	menuItem = new BMenuItem(B_TRANSLATE("About PonpokoDiff"), new BMessage(B_ABOUT_REQUESTED));
	menuItem->SetTarget(be_app_messenger);
	fileMenu->AddItem(menuItem);

	fileMenu->AddSeparatorItem();

	menuItem = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(MSG_FILE_QUIT), 'Q');
	fileMenu->AddItem(menuItem);
}


void
DiffWindow::_StartNodeMonitor()
{
	BEntry entry(fPathLeft.Path(), true);
	if (entry.InitCheck() == B_OK)
		entry.GetNodeRef(&fLeftNodeRef);

	entry.SetTo(fPathRight.Path(), true);
	if (entry.InitCheck() == B_OK)
		entry.GetNodeRef(&fRightNodeRef);

	watch_node(&fLeftNodeRef, B_WATCH_STAT | B_WATCH_NAME | B_WATCH_MOUNT, this);
	watch_node(&fRightNodeRef, B_WATCH_STAT | B_WATCH_NAME | B_WATCH_MOUNT, this);
}


void
DiffWindow::_HandleNodeMonitorEvent(BMessage* message)
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
				_AskToReload(nref);

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
				watch_node(&fLeftNodeRef, B_WATCH_STAT | B_WATCH_NAME | B_WATCH_MOUNT, this);
			} else if (strcmp(oldName, fPathRight.Leaf()) == 0) {
				fPathRight.SetTo(&entry);
				watch_node(&fRightNodeRef, B_STOP_WATCHING, this); // stop watching old file
				entry.GetNodeRef(&fRightNodeRef);
				watch_node(&fRightNodeRef, B_WATCH_STAT | B_WATCH_NAME | B_WATCH_MOUNT, this);
			} else
				break;

			_UpdateTitle();
			_StartNodeMonitor();
		} break;

		case B_ENTRY_REMOVED:
		{
			node_ref nref;

			if (message->FindInt32("device", &nref.device) != B_OK
				|| message->FindInt64("node", &nref.node) != B_OK)
				break;

			_AskFileRemoved(nref);
		} break;

		case B_DEVICE_UNMOUNTED:
		{
			node_ref nref;

			if (message->FindInt32("device", &nref.device) != B_OK)
				break;

			if (nref.device == fLeftNodeRef.device && nref.device == fRightNodeRef.device)
				_AskDeviceRemoved(BOTH);
			else if (nref.device == fLeftNodeRef.device)
				_AskDeviceRemoved(LEFT);
			else if (nref.device == fRightNodeRef.device)
				_AskDeviceRemoved(RIGHT);
		} break;
	}
}


void
DiffWindow::_AskToReload(node_ref nref)
{
	BString text;
	if (nref == fLeftNodeRef) {
		text = B_TRANSLATE(
			"The left file, '%filename%', has changed.");
		text.ReplaceFirst("%filename%", fPathLeft.Leaf());
	} else if (nref == fRightNodeRef) {
		text = B_TRANSLATE(
			"The right file, '%filename%', has changed.");
		text.ReplaceFirst("%filename%", fPathRight.Leaf());
	} else
		return;

	text << "\n\n" << B_TRANSLATE("Do you want to reload the files and diff them again?");
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
DiffWindow::_AskFileRemoved(node_ref nref)
{
	BString text;
	if (nref == fLeftNodeRef) {
		text = B_TRANSLATE(
			"The left file, '%filename%', has disappeared. Probably it was deleted "
			"or moved to another volume.");
		text.ReplaceFirst("%filename%", fPathLeft.Leaf());
	} else if (nref == fRightNodeRef) {
		text = B_TRANSLATE(
			"The right file, '%filename%', has disappeared. Probably it was deleted "
			"or moved to another volume.");
		text.ReplaceFirst("%filename%", fPathRight.Leaf());
	} else
		return;

	text << "\n\n" << B_TRANSLATE("Do you want to diff two new files, or just ignore this?");
	BAlert* alert = new BAlert(B_TRANSLATE("A file has disappeared"), text,
		B_TRANSLATE("Diff new files"), B_TRANSLATE("Ignore"));
	int32 result = alert->Go();

	switch(result) {
		case 0:
		{
			watch_node(&fLeftNodeRef, B_STOP_WATCHING, this); // stop watching old file
			watch_node(&fRightNodeRef, B_STOP_WATCHING, this); // stop watching old file

			BMessage message(MSG_FILE_OPEN);
			message.AddMessenger("killme", this);
			be_app->PostMessage(&message);

		} break;

		case 1:
			return;
	}
}


void
DiffWindow::_AskDeviceRemoved(pane_side side)
{
	BString text;
	if (side == BOTH) {
		text = B_TRANSLATE(
			"The volume of the files that are being compared has disappeared.");
	} else if (side == LEFT) {
		text = B_TRANSLATE(
			"The volume of the left file, '%filename%', has disappeared.");
		text.ReplaceFirst("%filename%", fPathLeft.Leaf());
	} else if (side == RIGHT) {
		text = B_TRANSLATE(
			"The volume of the right file, '%filename%', has disappeared.");
		text.ReplaceFirst("%filename%", fPathRight.Leaf());
	} else
		return;

	text << " " << B_TRANSLATE("Probably it was unmounted.") << "\n\n"
	<< B_TRANSLATE("Do you want to diff two new files, or just ignore this?");

	BAlert* alert = new BAlert(B_TRANSLATE("A file has disappeared"), text,
		B_TRANSLATE("Diff new files"), B_TRANSLATE("Ignore"));
	int32 result = alert->Go();

	switch(result) {
		case 0:
		{
			watch_node(&fLeftNodeRef, B_STOP_WATCHING, this); // stop watching old file
			watch_node(&fRightNodeRef, B_STOP_WATCHING, this); // stop watching old file

			BMessage message(MSG_FILE_OPEN);
			message.AddMessenger("killme", this);
			be_app->PostMessage(&message);

		} break;

		case 1:
			return;
	}
}


void
DiffWindow::_OpenFile(BPath path)
{
	entry_ref ref;
	get_ref_for_path(path.Path(), &ref);
	be_roster->Launch(&ref);
}


void
DiffWindow::_OpenLocation(BPath path)
{
	entry_ref ref;
	path.GetParent(&path);
	get_ref_for_path(path.Path(), &ref);
	be_roster->Launch(&ref);
}


void
DiffWindow::_UpdateTitle()
{
	BString title(B_TRANSLATE_SYSTEM_NAME("PonpokoDiff"));
	title += " : ";
	title += fPathLeft.Leaf();
	title += " ◄ | ► ";
	title += fPathRight.Leaf();

	SetTitle(title.String());
}


void
DiffWindow::_DoFileQuit()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
}
