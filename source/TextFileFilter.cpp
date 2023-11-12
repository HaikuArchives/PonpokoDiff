/*
 * Copyright 2023, Humdinger
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		Humdinger
 *
 */

#include "TextFileFilter.h"

#include <Node.h>
#include <NodeInfo.h>
#include <Volume.h>

#include <stdlib.h>


bool
TextFileFilter::Filter(const entry_ref* ref, BNode* node, struct stat_beos* stat,
	const char* fileType)
{
	BEntry entry(ref, true); // traverse links

	// allow folders and links of folders
	if (entry.IsDirectory())
		return true;

	if (IsValid(ref, &entry))
		return true;

	return false;
}


bool
TextFileFilter::IsValid(const entry_ref* ref, const BEntry* entry)
{
	// allow text and linked text files and catkeys
	char mimeType[B_MIME_TYPE_LENGTH];
	BNode traversedNode(entry); // create a new node from the link-traversed BEntry
	BNodeInfo(&traversedNode).GetType(mimeType);
	if (strncmp("text/", mimeType, 5) == 0)
		return true;

	if (strncmp("locale/x-vnd.Be.locale-catalog.plaintext", mimeType, 40) == 0)
		return true;

	// allow all, if volume doesn't know MIME
	BVolume volume;
	volume.SetTo((ref->device));
	if (volume.KnowsMime() == false)
		return true;

	return false;
}
