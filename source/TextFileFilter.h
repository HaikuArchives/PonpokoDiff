/*
 * Copyright 2023, Humdinger
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		Humdinger
 *
 */
#ifndef TEXTFILEFILTER_H_H
#define TEXTFILEFILTER_H


#include <FilePanel.h>
#include <SupportDefs.h>


class TextFileFilter : public BRefFilter {
public:
	bool				Filter(const entry_ref* entryRef, BNode* node,
							struct stat_beos* stat, const char* fileType);

	bool				IsValid(const entry_ref* entryRef, const BEntry* entry);
};

#endif // TEXTFILEFILTER_H_H
