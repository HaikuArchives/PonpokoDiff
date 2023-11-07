/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef EXCEPTION_H__INCLUDED
#define EXCEPTION_H__INCLUDED

#include <Path.h>


class Exception {
public:
						Exception(int code) { this->code = code; }
	virtual				~Exception() {}

			int			GetCode() const { return code; }
	virtual	void		Delete() { delete this; }

private:
			int			code;
};


class MemoryException : public Exception {
public:
	static void			Throw() { throw &theOnlyInstance; }
	virtual	void		Delete() { /* do nothing */ }

private:
						MemoryException();
	virtual				~MemoryException() {}

private:
	static MemoryException	theOnlyInstance;
};


class FileException : public Exception {
public:
						FileException(int code, const BPath& aPath, status_t aStatus = B_NO_ERROR)
							: Exception(code), path(aPath), status(aStatus) {}
	virtual				~FileException() {}

	const BPath&		GetPath() const { return path; }
	status_t			GetStatus() const { return status; }

private:
	BPath				path;
	status_t			status;
};

#endif // EXCEPTION_H__INCLUDED
