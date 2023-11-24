/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */
#ifndef DIFFWINDOW_H
#define DIFFWINDOW_H


#include <Path.h>
#include <Window.h>

#include "DiffView.h"

class BMenuBar;

enum pane_side {
	LEFT = 0,
	RIGHT,
	BOTH
};

class DiffWindow : public BWindow {
public:
							DiffWindow(BMessage* settings, int32 windowCount);
	virtual					~DiffWindow();

			void			ExecuteDiff(const BPath pathLeft, const BPath pathRight);

public:
	virtual	void			Quit();
	virtual	void			MessageReceived(BMessage* message);

private:
			void			_CreateMainMenu(BMenuBar* menuBar);
			void			_StartNodeMonitor();
			void			_HandleNodeMonitorEvent(BMessage* message);
			void			_AskToReload(node_ref nref);
			void			_AskFileRemoved(node_ref nref);
			void			_AskDeviceRemoved(pane_side side);
			void			_OpenFile(BPath path);
			void			_OpenLocation(BPath path);

			void			_SaveSettings();

			void			_UpdateTitle();
			void			_DoFileQuit();

			DiffView* 		fDiffView;
			BPath			fPathLeft;
			BPath			fPathRight;
			node_ref 		fLeftNodeRef;
			node_ref 		fRightNodeRef;
};

#endif // DIFFWINDOW_H
