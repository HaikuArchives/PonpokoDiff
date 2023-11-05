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
 *	@file		LineSeparatedText.cpp
 *	@brief		LineSeparatedText クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-19 Created
 */

#include "LineSeparatedText.h"
#include "Exception.h"
#include "ExceptionCode.h"

#include <File.h>
#include <Path.h>

#include <stdlib.h>

/**
 *	@brief	コンストラクタ
 */
LineSeparatedText::LineSeparatedText()
{
	loadedBuffer = NULL;
}

/**
 *	@brief	デストラクタ
 */
LineSeparatedText::~LineSeparatedText()
{
	Unload();
}

/**
 *	@brief 指定したファイルを読み込んで内容を保持します。
 *	@param[in]	path	ファイルパス
 */
void
LineSeparatedText::Load(const BPath& path)
{
	Unload();

	BFile file;
	status_t st = file.SetTo(path.Path(), B_READ_ONLY);
	if (B_NO_ERROR != st)
		throw new FileException(EXCEPTION_FILE_OPEN, path, st);

	off_t size;
	file.GetSize(&size);
	uint32 size32 = static_cast<uint32>(size);
	loadedBuffer = static_cast<char*>(malloc(size32));
	if (NULL == loadedBuffer)
		MemoryException::Throw();
	file.Read(loadedBuffer, size32);

	splitBuffer(size32);
}

/**
 *	@brief	現在読み込んでいるデータを消去します。
 */
void
LineSeparatedText::Unload()
{
	if (NULL != loadedBuffer) {
		free(loadedBuffer);
		loadedBuffer = NULL;
	}
	lines.clear();
}

/**
 *	@brief	loadedBuffer の中を行で区切って保持します。
 *	@param[in]	size	loadedBuffer が指すデータのサイズ
 */
void
LineSeparatedText::splitBuffer(uint32 size)
{
	char* ptr = loadedBuffer;
	char* endBuffer = loadedBuffer + size;

	char* strBegin = ptr;
	for (; ptr < endBuffer; ptr++) {
		if (*ptr == '\n') {
			lines.push_back(Substring(strBegin, ptr + 1));
			strBegin = ptr + 1;
		} else if (*ptr == '\r' && (ptr + 1 >= endBuffer || *(ptr + 1) != '\n')) {
			lines.push_back(Substring(strBegin, ptr + 1));
			strBegin = ptr + 1;
		}
	}
	if (strBegin < endBuffer)
		lines.push_back(Substring(strBegin, endBuffer));
}
