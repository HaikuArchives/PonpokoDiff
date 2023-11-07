/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */
#ifndef PONPOKODIFFAPP_H__INCLUDED
#define PONPOKODIFFAPP_H__INCLUDED


#include <Application.h>

class TextDiffWnd;
class OpenFilesDialog;


class PonpokoDiffApp : public BApplication {
public:
							PonpokoDiffApp();
	virtual					~PonpokoDiffApp();

			TextDiffWnd*	NewTextDiffWnd();
			void			TextDiffWndQuit(TextDiffWnd* wnd);
			void			OpenFilesDialogClosed();

public:
	virtual	void			ReadyToRun();
	virtual void			AboutRequested();
	virtual void			ArgvReceived(int32 argc, char** argv);
	virtual void			RefsReceived(BMessage* message);
	virtual	void			MessageReceived(BMessage* message);

private:
			void			makeNewTextDiffWndRect(BRect& frameRect);
			void			doOpenFileDialog();

private:
		int32				textDiffWndCount;
		OpenFilesDialog*	openFilesDialog;
};

#endif // PONPOKODIFFAPP_H__INCLUDED
