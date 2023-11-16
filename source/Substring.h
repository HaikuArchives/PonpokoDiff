/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef SUBSTRING_H
#define SUBSTRING_H


class Substring {
public:
					Substring(char* begin, char* end);
					Substring(char* cstring);
					Substring(char* cstring, int maxLength);
					Substring(const Substring& other);

	Substring&		operator=(const Substring& other);

	const char*		Begin() const	{ return begin; }
	const char*		End() const		{ return end; }
	int				Length() const { return end - begin; }

	bool			IsSameString(const Substring& other) const;
	bool			operator==(const Substring& other) const
						{ return IsSameString(other); }
	bool			operator!=(const Substring& other) const
						{ return !IsSameString(other); }

private:
	char*			begin;
	char*			end;
};

#endif // SUBSTRING_H
