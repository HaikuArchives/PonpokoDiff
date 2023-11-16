/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef LINESEPARATEDTEXT_H
#define LINESEPARATEDTEXT_H

#include <vector>

#include "Substring.h"

#include <Path.h>


class LineSeparatedText {
public:
						LineSeparatedText();
	virtual				~LineSeparatedText();

			void		Load(const BPath& path);
			void		Unload();

			int			GetLineCount() const { return fLines.size(); }
	const Substring&	GetLineAt(int index) const { return fLines[index]; }

private:
			void		_SplitBuffer(uint32 size);

private:
	typedef std::vector<Substring>	SubstringVector;

			char*		fLoadedBuffer;
	SubstringVector		fLines;
};

#endif // LINESEPARATEDTEXT_H
