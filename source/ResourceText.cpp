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
 *	@file		ResourceText.cpp
 *	@brief		文字列リソースの取得用クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2008-01-06 Created
 */

#include "ResourceText.h"

/// 唯一の ResourceText オブジェクト
ResourceText ResourceText::theInstance;

/**
 *	@brief	コンストラクタ
 */
ResourceText::ResourceText()
{
	resStrs = new BResourceStrings();
}

/**
 *	@brief	デストラクタ
 */
ResourceText::~ResourceText()
{
	delete resStrs;
}

/**
 *	@brief	ID から文字列を得ます
 *	@param[in]	id	文字列リソースID
 *	@return	文字列。
 */
const char* ResourceText::FindString(int id)
{
	if (NULL == resStrs)
	{
		return NULL;
	}
	else
	{
		return resStrs->FindString(id);
	}
}
