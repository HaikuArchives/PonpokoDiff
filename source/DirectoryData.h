/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *		Adrien Destugues (PulkoMandy)
 *
 */
#ifndef DIRECTORYDATA_H
#define DIRECTORYDATA_H

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

			int			GetLineCount() const { return fLines.size(); }
	const Substring&	GetLineAt(int index) const { return fLines[index]; }

private:
	typedef std::vector<Substring>	SubstringVector;

	SubstringVector		fLines;
	std::vector<BString> fData;
};

#endif // DIRECTORYDATA_H
