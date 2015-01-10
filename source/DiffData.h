/*
 * PonpokoDiff
 *
 * Copyright (c) 2011 PonpokoDiff Project Contributors
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
 *	@file		DiffData.h
 *	@brief		Abstract class for diff data
 *	@author		Adrien Destugues (PulkoMandy)
 *	@date		2011-06-19 Created
 */

#ifndef DIFFDATA_H__INCLUDED
#define DIFFDATA_H__INCLUDED

#include <vector>
#include "Substring.h"

/**
 *	@brief	テキストの内容を行ごとに保持するクラス
 */
class DiffData
{
public:
						DiffData(const BPath& path) {};
	virtual				~DiffData() {};
	
	virtual void		Load(const BPath& path) = 0;
	virtual void		Unload() = 0;
	
	virtual int			GetLineCount() const = 0;
	virtual const Substring&	GetLineAt(int index) const = 0;
};

#endif // DIFFDATA_H__INCLUDED
