/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <Path.h>


class Exception {
public:
						Exception(int fCode) { this->fCode = fCode; }
	virtual				~Exception() {}

			int			GetCode() const { return fCode; }
	virtual	void		Delete() { delete this; }

private:
			int			fCode;
};


class MemoryException : public Exception {
public:
	static void			Throw() { throw &fTheOnlyInstance; }
	virtual	void		Delete() { /* do nothing */ }

private:
						MemoryException();
	virtual				~MemoryException() {}

private:
	static MemoryException	fTheOnlyInstance;
};


class FileException : public Exception {
public:
						FileException(int fCode, const BPath& aPath, status_t aStatus = B_NO_ERROR)
							: Exception(fCode), fPath(aPath), fStatus(aStatus) {}
	virtual				~FileException() {}

	const BPath&		GetPath() const { return fPath; }
	status_t			GetStatus() const { return fStatus; }

private:
	BPath				fPath;
	status_t			fStatus;
};

#endif // EXCEPTION_H
