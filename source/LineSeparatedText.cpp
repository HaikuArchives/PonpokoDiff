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
	fLoadedBuffer = NULL;
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
	fLoadedBuffer = static_cast<char*>(malloc(size32));
	if (fLoadedBuffer == NULL)
		MemoryException::Throw();
	file.Read(fLoadedBuffer, size32);

	_SplitBuffer(size32);
}


void
LineSeparatedText::Unload()
{
	if (fLoadedBuffer != NULL) {
		free(fLoadedBuffer);
		fLoadedBuffer = NULL;
	}
	fLines.clear();
}


void
LineSeparatedText::_SplitBuffer(uint32 size)
{
	char* ptr = fLoadedBuffer;
	char* endBuffer = fLoadedBuffer + size;

	char* strBegin = ptr;
	for (; ptr < endBuffer; ptr++) {
		if (*ptr == '\n') {
			fLines.push_back(Substring(strBegin, ptr + 1));
			strBegin = ptr + 1;
		} else if (*ptr == '\r' && (ptr + 1 >= endBuffer || *(ptr + 1) != '\n')) {
			fLines.push_back(Substring(strBegin, ptr + 1));
			strBegin = ptr + 1;
		}
	}
	if (strBegin < endBuffer)
		fLines.push_back(Substring(strBegin, endBuffer));
}
