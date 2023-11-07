/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Adrien Destugues (PulkoMandy)
 *
 */
#ifndef DIRECTORYDATA_H__INCLUDED
#define DIRECTORYDATA_H__INCLUDED

#include <vector>

#include "DiffData.h"
#include "Substring.h"


class DirectoryData: public DiffData
{
public:
						DirectoryData(const BPath& path);
	virtual				~DirectoryData();

			void		Load(const BPath& path);
			void		Unload();

			int			GetLineCount() const		{ return lines.size(); }
	const Substring&	GetLineAt(int index) const	{ return lines[index]; }

private:
	typedef std::vector<Substring>	SubstringVector;

	SubstringVector		lines;
	std::vector<BString> data;
};

#endif // LINESEPARATEDTEXT_H__INCLUDED
