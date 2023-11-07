/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Adrien Destugues (PulkoMandy)
 *
 */
#include "LineSeparatedText.h"
#include "Exception.h"
#include "ExceptionCode.h"

#include <File.h>
#include <Path.h>

#include <stdlib.h>


LineSeparatedText::LineSeparatedText()
{
	loadedBuffer = NULL;
}


LineSeparatedText::~LineSeparatedText()
{
	Unload();
}


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


void
LineSeparatedText::Unload()
{
	if (loadedBuffer != NULL) {
		free(loadedBuffer);
		loadedBuffer = NULL;
	}
	lines.clear();
}


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
