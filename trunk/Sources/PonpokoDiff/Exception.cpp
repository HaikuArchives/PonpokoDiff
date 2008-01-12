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
 *	@file		Exception.cpp
 *	@brief		例外クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2008-01-04 Created
 */

#include "Prefix.h"
#include "Exception.h"
#include "ExceptionCode.h"

/**
 *	@brief	メモリ不足例外の唯一のインスタンス
 *
 *	メモリ不足例外が投げられるときはメモリが不足しているときなので
 *	メモリ不足例外のインスタンスが作れないかもしれません。
 *	このため、メモリ不足例外は唯一のインスタンスを先に用意しておき、
 *	これを使い回します。
 */
MemoryException MemoryException::theOnlyInstance;

MemoryException::MemoryException()
	: Exception(EXCEPTION_MEMORY)
{
}
