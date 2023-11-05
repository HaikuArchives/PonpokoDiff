/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Mark Hellegers <mark@firedisk.net>
 */
#ifndef LOCATIONINPUT_H
#define LOCATIONINPUT_H


#include <TextControl.h>


class LocationInput : public BTextControl {
public:
							LocationInput(const char* name,	const char* label);
		virtual				~LocationInput();

		virtual void		MessageReceived(BMessage* message);
};

#endif // LOCATIONINPUT_H
