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
 *	@file		LineSeparatedText.h
 *	@brief		LineSeparatedText クラスの定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-17 Created
 */

#ifndef LINESEPARATEDTEXT_H__INCLUDED
#define LINESEPARATEDTEXT_H__INCLUDED

#include <vector>
#include "Substring.h"

/**
 *	@brief	テキストの内容を行ごとに保持するクラス
 */
class LineSeparatedText
{
public:
						LineSeparatedText();
	virtual				~LineSeparatedText();
	
	void				Load(const BPath& path);
	void				Unload();
	
	int					GetLineCount() const		{ return lines.size(); }
	const Substring&	GetLineAt(int index) const	{ return lines[index]; }

private:
	void				splitBuffer(uint32 size);

private:
	typedef std::vector<Substring>	SubstringVector;
	
	char*				loadedBuffer;	///< 読み込んだファイルの内容全体のバッファ
	SubstringVector		lines;			///< 1行ごとのデータ
};

#endif // LINESEPARATEDTEXT_H__INCLUDED
