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
 *	@file		Substring.cpp
 *	@brief		Substring クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-17 Created
 */

#include "Substring.h"
#include <string.h>

/**
 *	@brief	開始点と終了点を指定するコンストラクタ
 *	@param[in]	begin	開始点
 *	@param[in]	end		終了点 (部分文字列の最後の文字の次の場所を指定します)
 */
Substring::Substring(char* begin, char* end)
{
	this->begin = begin;
	this->end = end;
}

/**
 *	@brief	C文字列で指定するコンストラクタ
 *	@param[in]	cstring	C文字列
 */
Substring::Substring(char* cstring)
{
	this->begin = cstring;
	this->end = cstring + strlen(cstring);
}

/**
 *	@brief	C文字列と最大長で指定するコンストラクタ
 *	@param[in]	cstring		C文字列
 *	@param[in]	maxLength	最大長
 */
Substring::Substring(char* cstring, int maxLength)
{
	this->begin = cstring;
	int len = strlen(cstring);
	this->end = cstring + ((len > maxLength) ? maxLength : len);
}

/**
 *	@brief	コピーコンストラクタ
 *	@param[in]	other	コピー元
 */
Substring::Substring(const Substring& other)
{
	this->begin = other.begin;
	this->end = other.end;
}

/**
 *	@brief	代入演算子
 *	@param[in]	other	代入元
 */
Substring& Substring::operator=(const Substring& other)
{
	if (this != &other)
	{
		this->begin = other.begin;
		this->end = other.end;		
	}
	return *this;
}

/**
 *	@brief	指定した文字列と内容が同じかどうか調べます。
 *	@param[in]	other	比べる文字列
 *	@return	同じなら true。
 */
bool Substring::IsSameString(const Substring& other) const
{
	if (Length() != other.Length())
	{
		return false;
	}

	char* ch1;
	char* ch2;
	for (ch1 = begin, ch2 = other.begin;
		ch1 < end;
		ch1++, ch2++)
	{
		if (ch2 >= other.end)
		{
			return false;
		}
		if (*ch1 != *ch2)
		{
			return false;
		}
	}
	return true;
}
