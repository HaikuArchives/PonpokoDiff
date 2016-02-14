#ifndef LOCATIONINPUT_H
#define LOCATIONINPUT_H

// BeOS headers
#include <TextControl.h>

class LocationInput : public BTextControl
{
	public:
		LocationInput(BRect frame,
					  const char * name,
					  const char * label);
		virtual ~LocationInput();
		virtual void MessageReceived(BMessage * message);
};

#endif
