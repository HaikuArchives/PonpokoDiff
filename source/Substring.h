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
 *	@file		Substring.h
 *	@brief		Substring クラスの定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-17 Created
 */
#ifndef SUBSTRING_H__INCLUDED
#define SUBSTRING_H__INCLUDED

/**
 *	@brief	開始位置と終了位置で表される部分文字列のクラス。
 *			C文字列とは異なり、\0 終端ではありません。
 */
class Substring {
public:
					Substring(char* begin, char* end);
					Substring(char* cstring);
					Substring(char* cstring, int maxLength);
					Substring(const Substring& other);

	Substring&		operator=(const Substring& other);

	const char*		Begin() const	{ return begin; }
	const char*		End() const		{ return end; }
	int				Length() const { return end - begin; }

	bool			IsSameString(const Substring& other) const;
	bool			operator==(const Substring& other) const
						{ return IsSameString(other); }
	bool			operator!=(const Substring& other) const
						{ return !IsSameString(other); }

private:
	char*			begin;
	char*			end;
};

#endif // SUBSTRING_H__INCLUDED
