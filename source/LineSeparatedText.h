/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef LINESEPARATEDTEXT_H__INCLUDED
#define LINESEPARATEDTEXT_H__INCLUDED

#include <vector>

#include "Substring.h"

#include <Path.h>


class LineSeparatedText {
public:
						LineSeparatedText();
	virtual				~LineSeparatedText();

			void		Load(const BPath& path);
			void		Unload();

			int			GetLineCount() const { return lines.size(); }
	const Substring&	GetLineAt(int index) const { return lines[index]; }

private:
			void		splitBuffer(uint32 size);

private:
	typedef std::vector<Substring>	SubstringVector;

			char*		loadedBuffer;
	SubstringVector		lines;
};

#endif // LINESEPARATEDTEXT_H__INCLUDED
