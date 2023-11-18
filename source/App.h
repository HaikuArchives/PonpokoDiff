/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */
#ifndef APP_H
#define APP_H


#include <Application.h>

class DiffWindow;
class OpenFilesDialog;


class App : public BApplication {
public:
							App();
	virtual					~App();

			DiffWindow*		NewDiffWindow();
			void			DiffWindowQuit(DiffWindow* window);
			void			OpenFilesPanelClosed();

public:
	virtual	void			ReadyToRun();
	virtual void			AboutRequested();
	virtual void			ArgvReceived(int32 argc, char** argv);
	virtual void			RefsReceived(BMessage* message);
	virtual	void			MessageReceived(BMessage* message);

private:
			void			_HelpWindow();
			void			_LoadSettings();
			void			_OpenFilesPanel();

private:
		BMessage*			fSettings;
		int32				fWindowCount;
		OpenFilesDialog*	fOpenFilesPanel;
};

#endif // APP_H
