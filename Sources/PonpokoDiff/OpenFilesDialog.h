/*
 * PonpokoDiff
 *
 * Copyright (c) 2008 PonpokoDiff Project Contributors
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
/**
 *	@file		OpenFilesDialog.h
 *	@brief		OpenFilesDialog クラスの定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *  @date		2008-01-06 Created
 */

#ifndef OPENFILESDIALOG_H__INCLUDED
#define OPENFILESDIALOG_H__INCLUDED

#include <Window.h>
#include <FilePanel.h>

/**
 *	@brief	比較するファイルを選択するダイアログ
 */
class OpenFilesDialog : public BWindow
{
public:
						OpenFilesDialog(BPoint topLeft);
	virtual				~OpenFilesDialog();
	
	void				Initialize();

public:
	virtual	void		Quit();
	virtual	void		MessageReceived(BMessage* message);

private:
	enum FileIndex
	{
		LeftFile = 0,	///< 左ファイル
		RightFile,		///< 右ファイル
		
		FileMAX			/// ファイル最大数
	};
	
private:
	void				doBrowseFile(OpenFilesDialog::FileIndex fileIndex);
	void				doFileSelected(OpenFilesDialog::FileIndex fileIndex, BMessage* message);
	void				doDiffThem();
	
private:
	BFilePanel*			filePanels[FileMAX];	///< ファイルを選択するためのファイルパネル
};

#endif // OPENFILESDIALOG_H__INCLUDED
