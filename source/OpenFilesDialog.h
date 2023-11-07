/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef OPENFILESDIALOG_H__INCLUDED
#define OPENFILESDIALOG_H__INCLUDED

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
		LeftFile = 0,
		RightFile,

		FileMAX
	};

private:
			void		doBrowseFile(OpenFilesDialog::FileIndex fileIndex);
			void		doFileSelected(OpenFilesDialog::FileIndex fileIndex, BMessage* message);
			void		doDiffThem();

private:
		BFilePanel*		filePanels[FileMAX];
};

#endif // OPENFILESDIALOG_H__INCLUDED
