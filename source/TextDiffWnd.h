/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */
#ifndef TEXTDIFFWND_H__INCLUDED
#define TEXTDIFFWND_H__INCLUDED


#include <Path.h>
#include <Window.h>


class BMenuBar;

class TextDiffWnd : public BWindow {
public:
							TextDiffWnd(BRect frame, const char* name,
								uint32 workspaces = B_CURRENT_WORKSPACE);
	virtual					~TextDiffWnd();

			void			Initialize();

			void			ExecuteDiff(const BPath& pathLeft, const BPath& pathRight);

public:
	virtual	void			Quit();
	virtual	void			MessageReceived(BMessage* message);

private:
			void			createMainMenu(BMenuBar* menuBar);
			void			startNodeMonitor();
			void			handleNodeMonitorEvent(BMessage* message);
			void			askToReload(node_ref nref_node);
			void			updateTitle();
			void			doFileOpen();
			void			doFileQuit();

			BPath			fPathLeft;
			BPath			fPathRight;
			node_ref 		fLeftNodeRef;
			node_ref 		fRightNodeRef;

			const char*		fLabelLeft;
			const char*		fLabelRight;
};

#endif // TEXTDIFFWND_H__INCLUDED
