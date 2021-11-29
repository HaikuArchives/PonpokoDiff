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
 *	@file		PonpokoDiffApp.h
 *	@brief		PonpokoDiffApp クラスの定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-16 Created
 */

#ifndef PONPOKODIFFAPP_H__INCLUDED
#define PONPOKODIFFAPP_H__INCLUDED

#include <Application.h>

class TextDiffWnd;
class OpenFilesDialog;

/**
 *	@brief	PonpokoDiff アプリケーションクラス
 */
class PonpokoDiffApp : public BApplication
{
public:
						PonpokoDiffApp();
	virtual				~PonpokoDiffApp();

	TextDiffWnd*		NewTextDiffWnd();
	void				TextDiffWndQuit(TextDiffWnd* wnd);
	void				OpenFilesDialogClosed();
	
public:
	virtual	void		ReadyToRun();
	virtual void		AboutRequested();
	virtual void		ArgvReceived(int32 argc, char** argv);
	virtual void		RefsReceived(BMessage* message);
	virtual	void		MessageReceived(BMessage* message);

private:
	void				makeNewTextDiffWndRect(BRect& frameRect);
	void				doOpenFileDialog();
	void				makeVersionString(BString& versionString);

private:
	int32				textDiffWndCount;		///< 動作している TextDiffWnd の数
	OpenFilesDialog*	openFilesDialog;		///< 前回表示したファイルを開くダイアログ
};

#endif // PONPOKODIFFAPP_H__INCLUDED
