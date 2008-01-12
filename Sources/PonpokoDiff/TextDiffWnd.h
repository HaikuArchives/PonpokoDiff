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
 *	@file		TextDiffWnd.h
 *	@brief		TextDiffWnd クラスの定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *  @date		2007-12-24 Created
 */

#ifndef TEXTDIFFWND_H__INCLUDED
#define TEXTDIFFWND_H__INCLUDED

#include <Window.h>

class BPath;
class BMenuBar;

/**
 *	@brief	テキストの差分を表示するウィンドウ
 */
class TextDiffWnd : public BWindow
{
public:
						TextDiffWnd(BRect frame, const char* name, uint32 workspaces = B_CURRENT_WORKSPACE);
	virtual				~TextDiffWnd();

	void				Initialize();
	
	void				ExecuteDiff(const BPath& pathLeft, const BPath& pathRight, const char* labelLeft, const char* labelRight);

public:
	virtual	void		Quit();
	virtual	void		MessageReceived(BMessage* message);

private:
	void				createMainMenu(BMenuBar* menuBar);
	void				doFileOpen();
	void				doFileQuit();
};

#endif // TEXTDIFFWND_H__INCLUDED


