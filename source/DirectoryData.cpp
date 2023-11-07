/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Adrien Destugues (PulkoMandy)
 *
 */
#include "DirectoryData.h"
#include "Exception.h"
#include "ExceptionCode.h"
#include <Directory.h>
#include <File.h>
#include <Path.h>
#include <String.h>


DirectoryData::DirectoryData(const BPath& path)
	:
	DiffData(path)
{
	Load(path);
}


DirectoryData::~DirectoryData()
{
	Unload();
}


void
DirectoryData::Load(const BPath& path)
{
	Unload();

	BDirectory dir(path.Path());

	BEntry current;
	status_t status;

	while ((status = dir.GetNextEntry(&current)) != B_ENTRY_NOT_FOUND) {
		BPath path;
		current.GetPath(&path);
		BString s(path.Path());

		struct stat fileInfo;
		current.GetStat(&fileInfo);

		s.Append("\t");
		s << fileInfo.st_size;

		data.push_back(s);
		lines.push_back(Substring(s));
	}

	// splitBuffer(size32);
}


void
DirectoryData::Unload()
{
	lines.clear();
	data.clear();
}
