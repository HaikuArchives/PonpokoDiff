/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Humdinger
 *
 */
#ifndef OPENFILESDIALOG_H
#define OPENFILESDIALOG_H

#include "LocationInput.h"

#include <FilePanel.h>
#include <Window.h>


class OpenFilesDialog : public BWindow {
public:
						OpenFilesDialog(BMessage* fSettings);
	virtual				~OpenFilesDialog();

	virtual	void		Quit();
	virtual	void		MessageReceived(BMessage* message);

private:
	enum FileIndex {
		LEFTFILE = 0,
		RIGHTFILE,

		FileMAX
	};

private:
			void		_Initialize();
			void		_BrowseFile(OpenFilesDialog::FileIndex fileIndex);
			void		_FileSelected(OpenFilesDialog::FileIndex fileIndex,
							BMessage* message);
			void		_RunDiff();

private:
		BFilePanel*		fFilePanels[FileMAX];
		LocationInput*	fLeftLocation;
		LocationInput*	fRightLocation;
};

#endif // OPENFILESDIALOG_H
