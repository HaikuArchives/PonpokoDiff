/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#include "Exception.h"
#include "ExceptionCode.h"


MemoryException MemoryException::fTheOnlyInstance;


MemoryException::MemoryException()
	:
	Exception(EXCEPTION_MEMORY)
{
}
