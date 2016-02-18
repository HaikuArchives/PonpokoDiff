/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Mark Hellegers <mark@firedisk.net>
 */


#include "LocationInput.h"

// BeOS headers
#include <Entry.h>
#include <Path.h>


LocationInput::LocationInput(BRect frame,
	const char* name,
	const char* label)
	:
	BTextControl(frame, name, label, "", NULL)
{
}

LocationInput::~LocationInput()
{
}

void LocationInput::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SIMPLE_DATA:
		{
			// Get the ref that is dropped.
			entry_ref ref;
			message->FindRef("refs", &ref);
			BEntry entry(&ref);
			BPath path;
			entry.GetPath(&path);
			SetText(path.Path());
			break;
		}
		default:
			BTextControl::MessageReceived(message);
			break;
	}
	
}
