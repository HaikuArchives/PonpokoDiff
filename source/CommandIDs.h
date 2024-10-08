/*
 * Copyright 2008, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef COMMANDIDS_H
#define COMMANDIDS_H

#include "AppDefs.h"


enum
{
	MSG_FILE_OPEN			= 'mOpn',
	MSG_FILE_NEW			= 'mNew',
	MSG_FILE_RELOAD			= 'mRld',
	MSG_FILE_QUIT			= 'mQit',
	MSG_FILE_DROPPED		= 'mDrp',
	MSG_FILE_LAUNCH			= 'mLnc',
	MSG_FILE_SWITCH			= 'mSwi',

	MSG_OPEN_LOCATION		= 'mLoc',
	MSG_HELP				= 'mhlp',

	MSG_CANCEL				= '-Ccl',

	MSG_OFD_BROWSE_LEFT		= 'fBrL',
	MSG_OFD_BROWSE_RIGHT	= 'fBrR',
	MSG_OFD_DIFF_THEM		= 'fDif',
	MSG_OFD_LEFT_SELECTED	= 'fSlL',
	MSG_OFD_RIGHT_SELECTED	= 'fSlR',
};

#endif // COMMANDIDS_H
