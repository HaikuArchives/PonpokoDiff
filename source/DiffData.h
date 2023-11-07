/*
 * Copyright 2015, Adrien Destugues (PulkoMandy)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		Adrien Destugues (PulkoMandy)
 *
 */
#ifndef DIFFDATA_H__INCLUDED
#define DIFFDATA_H__INCLUDED

#include <vector>

#include "Substring.h"


class DiffData {
public:
						DiffData(const BPath& path) {};
	virtual				~DiffData() {};

	virtual void		Load(const BPath& path) = 0;
	virtual void		Unload() = 0;

	virtual int			GetLineCount() const = 0;
	virtual const Substring&	GetLineAt(int index) const = 0;
};

#endif // DIFFDATA_H__INCLUDED
