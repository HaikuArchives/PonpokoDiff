/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Mark Hellegers <mark@firedisk.net>
 */


#include "LocationInput.h"
#include "TextFileFilter.h"

#include <Entry.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Volume.h>


LocationInput::LocationInput(const char* name, const char* label)
	:
	BTextControl(name, label, "", NULL)
{
}


LocationInput::~LocationInput()
{
}


void
LocationInput::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SIMPLE_DATA:
		{
			// Get the ref that is dropped.
			entry_ref ref;
			message->FindRef("refs", &ref);

			BEntry entry(&ref, true);
			TextFileFilter filter;
			if (filter.IsValid(&ref, &entry))
				MarkAsInvalid(false);
			else {
				BString text = Text();
				// only invalid if there wasn't a valid file before
				if (text == "")
					MarkAsInvalid(true);
				break;
			}

			BPath path;
			entry.GetPath(&path);
			SetText(path.Path());
		} break;

		default:
			BTextControl::MessageReceived(message);
			break;
	}
}