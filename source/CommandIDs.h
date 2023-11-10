/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef COMMANDIDS_H__INCLUDED
#define COMMANDIDS_H__INCLUDED

#include "AppDefs.h"


enum
{
	ID_FILE_OPEN			= 'mOpn',
	ID_FILE_RELOAD			= 'mRld',
	ID_FILE_CLOSE			= B_QUIT_REQUESTED,
	ID_FILE_ABOUT			= B_ABOUT_REQUESTED,
	ID_FILE_QUIT			= 'mQit',
	ID_FILE_DROPPED			= 'mDrp',
	ID_FILE_LAUNCH			= 'mlnc',

	ID_CANCEL				= '-Ccl',

	ID_OFD_BROWSE_LEFT		= 'fBrL',
	ID_OFD_BROWSE_RIGHT		= 'fBrR',
	ID_OFD_DIFF_THEM		= 'fDif',
	ID_OFD_LEFT_SELECTED	= 'fSlL',
	ID_OFD_RIGHT_SELECTED	= 'fSlR',
};

#endif // COMMANDIDS_H__INCLUDED
