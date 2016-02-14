// BeOS headers
#include <Path.h>
#include <Entry.h>

// PonpokoDiff headers
#include "LocationInput.h"

LocationInput :: LocationInput(BRect frame,
							   const char * name,
							   const char * label)
			  : BTextControl(frame, name, label, "", NULL)
{
}

LocationInput :: ~LocationInput()
{
}

void LocationInput :: MessageReceived(BMessage * message)
{
	switch (message->what)
	{
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
