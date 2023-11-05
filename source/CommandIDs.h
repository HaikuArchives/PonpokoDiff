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
 *	@file		CommandIDs.h
 *	@brief		コマンド ID の定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2008-01-06 Created
 */

#ifndef COMMANDIDS_H__INCLUDED
#define COMMANDIDS_H__INCLUDED

#include "AppDefs.h"

/// コマンドID
enum
{
	// メニュー
	ID_FILE_OPEN			= 'mOpn',
	ID_FILE_RELOAD			= 'mRld',
	ID_FILE_CLOSE			= B_QUIT_REQUESTED,
	ID_FILE_ABOUT			= B_ABOUT_REQUESTED,
	ID_FILE_QUIT			= 'mQit',
	
	// ダイアログ
//	ID_OK					= '-Ok!',
	ID_CANCEL				= '-Ccl',
	
	// ファイルを選択するダイアログ
	ID_OFD_BROWSE_LEFT		= 'fBrL',
	ID_OFD_BROWSE_RIGHT		= 'fBrR',
	ID_OFD_DIFF_THEM		= 'fDif',
	ID_OFD_LEFT_SELECTED	= 'fSlL',
	ID_OFD_RIGHT_SELECTED	= 'fSlR',
};

#endif // COMMANDIDS_H__INCLUDED
