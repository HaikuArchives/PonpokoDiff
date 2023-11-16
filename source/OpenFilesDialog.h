/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef OPENFILESDIALOG_H
#define OPENFILESDIALOG_H

#include <FilePanel.h>
#include <Window.h>


class OpenFilesDialog : public BWindow {
public:
						OpenFilesDialog(BPoint topLeft);
	virtual				~OpenFilesDialog();

			void		Initialize();

public:
	virtual	void		Quit();
	virtual	void		MessageReceived(BMessage* message);

private:
	enum FileIndex {
		LEFTFILE = 0,
		RIGHTFILE,

		FileMAX
	};

private:
			void		_BrowseFile(OpenFilesDialog::FileIndex fileIndex);
			void		_FileSelected(OpenFilesDialog::FileIndex fileIndex, BMessage* message);
			void		_RunDiff();

private:
		BFilePanel*		fFilePanels[FileMAX];
};

#endif // OPENFILESDIALOG_H
