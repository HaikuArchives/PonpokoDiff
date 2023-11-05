/*
 * PonpokoDiff
 *
 * Copyright (c) 2008 PonpokoDiff Project Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 *	@file		Exception.h
 *	@brief		例外クラスの定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2008-01-04 Created
 */
#ifndef EXCEPTION_H__INCLUDED
#define EXCEPTION_H__INCLUDED

#include <Path.h>

/**
 *	@brief	例外の基底クラス
 */
class Exception {
public:
						Exception(int code) { this->code = code; }
	virtual				~Exception() {}

			int			GetCode() const { return code; }
	virtual	void		Delete() { delete this; }

private:
			int			code;		///< 例外コード
};

/**
 *	@brief メモリ不足例外クラス
 */
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

/**
 *	@brief	ファイル関連の例外クラス
 */
class FileException : public Exception {
public:
						FileException(int code, const BPath& aPath, status_t aStatus = B_NO_ERROR)
							: Exception(code), path(aPath), status(aStatus) {}
	virtual				~FileException() {}

	const BPath&		GetPath() const { return path; }
	status_t			GetStatus() const { return status; }

private:
	BPath				path;	///< 例外の発生したファイルのパス
	status_t			status;	///< 例外のステータス
};

#endif // EXCEPTION_H__INCLUDED
