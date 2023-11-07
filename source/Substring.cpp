/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#include "Substring.h"
#include <string.h>


Substring::Substring(char* begin, char* end)
{
	this->begin = begin;
	this->end = end;
}


Substring::Substring(char* cstring)
{
	this->begin = cstring;
	this->end = cstring + strlen(cstring);
}


Substring::Substring(char* cstring, int maxLength)
{
	this->begin = cstring;
	int len = strlen(cstring);
	this->end = cstring + ((len > maxLength) ? maxLength : len);
}


Substring::Substring(const Substring& other)
{
	this->begin = other.begin;
	this->end = other.end;
}


Substring&
Substring::operator=(const Substring& other)
{
	if (this != &other) {
		this->begin = other.begin;
		this->end = other.end;
	}
	return *this;
}


bool
Substring::IsSameString(const Substring& other) const
{
	if (Length() != other.Length())
		return false;

	char* ch1;
	char* ch2;
	for (ch1 = begin, ch2 = other.begin; ch1 < end; ch1++, ch2++) {
		if (ch2 >= other.end)
			return false;
		if (*ch1 != *ch2)
			return false;
	}
	return true;
}
