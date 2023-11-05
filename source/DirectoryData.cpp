/*
 * PonpokoDiff
 *
 * Copyright (c) 2008-2011 PonpokoDiff Project Contributors
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
 *	@file		DirectoryData.cpp
 *	@brief		DirectoryData クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@author		Adrien Destugues (PulkoMandy)
 *	@date		2007-12-19 Created
 */

#include "DirectoryData.h"
#include <File.h>
#include <Directory.h>
#include <Path.h>
#include <String.h>
#include "Exception.h"
#include "ExceptionCode.h"

/**
 *	@brief	コンストラクタ
 */
DirectoryData::DirectoryData(const BPath& path)
	: DiffData(path)
{
	Load(path);
}

/**
 *	@brief	デストラクタ
 */
DirectoryData::~DirectoryData()
{
	Unload();
}

/**
 *	@brief 指定したファイルを読み込んで内容を保持します。
 *	@param[in]	path	ファイルパス
 */
void DirectoryData::Load(const BPath& path)
{
	Unload();
	
	BDirectory dir(path.Path());

	BEntry current;
	status_t status;

	while((status = dir.GetNextEntry(&current)) != B_ENTRY_NOT_FOUND)
	{
		BPath path;
		current.GetPath(&path);
		BString s(path.Path());

		struct stat fileInfo;	
		current.GetStat(&fileInfo);

		s.Append("\t");
		s<< fileInfo.st_size;

		data.push_back(s);
		lines.push_back(Substring(s));	
	}
	
	//splitBuffer(size32);
}

/**
 *	@brief	現在読み込んでいるデータを消去します。
 */
void DirectoryData::Unload()
{
	lines.clear();
	data.clear();
}

