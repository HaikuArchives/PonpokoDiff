/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#include "NPDiff.h"
#include "Exception.h"
#include "ExceptionCode.h"

#include <stdlib.h>
#include <string.h>


NPDiff::NPDiff()
{
	fp = NULL;
	fpBuffer = NULL;
	sequences = NULL;
	isSwapped = false;
}


NPDiff::~NPDiff()
{
}


void
NPDiff::Detect(const Sequences* sequences)
{
	this->sequences = sequences;
	isSwapped = (sequences->GetLength(0) > sequences->GetLength(1));
	diffResult.clear();

	if (NULL == sequences)
		return;

	int m = getLength(0);
	int n = getLength(1);
	int delta = n - m;

	fpBuffer = static_cast<int*>(malloc((m + n + 3) * sizeof(int)));
	if (NULL == fpBuffer)
		MemoryException::Throw();

	memset(fpBuffer, 0, (m + n + 3) * sizeof(int));
	fp = fpBuffer + m + 1;

	int p;
	for (p = 0; p < m; p++) // p < m は念のため
	{
		int k;
		for (k = -p; k <= delta - 1; k++)
			snake(k);
		for (k = delta + p; k >= delta; k--)
			snake(k);

		int fpDataIndexDelta = fp[delta] - 1;
		int fpDelta = (0 > fpDataIndexDelta) ? -1 : fpDataVector[fpDataIndexDelta].y;
		if (fpDelta == n)
			break;
	}

	makeResult();

	free(fpBuffer);
	fpBuffer = fp = NULL;
	fpDataVector.clear();
}


void
NPDiff::snake(int k)
{
	int fpDataIndex0 = fp[k - 1] - 1;
	int fpDataIndex1 = fp[k + 1] - 1;

	int fpY0 = (0 > fpDataIndex0) ? -1 : fpDataVector[fpDataIndex0].y;
	int fpY1 = (0 > fpDataIndex1) ? -1 : fpDataVector[fpDataIndex1].y;

	FPData data;
	if (fpY0 + 1 > fpY1) {
		data.y = fpY0 + 1;
		data.prevFPDataIndex = fpDataIndex0;
	} else {
		data.y = fpY1;
		data.prevFPDataIndex = fpDataIndex1;
	}
	data.x = data.y - k;

	while (data.x < getLength(0) && data.y < getLength(1) && isEqual(data.x, data.y)) {
		data.x++;
		data.y++;
	}

	fpDataVector.push_back(data);
	fp[k] = static_cast<int>(fpDataVector.size());
}


void
NPDiff::makeResult()
{
	int fpDataIndex = fpDataVector.size() - 1;
	const FPData* data = &fpDataVector[fpDataIndex];
	int to0 = (isSwapped) ? data->y : data->x;
	int to1 = (isSwapped) ? data->x : data->y;
	fpDataIndex = data->prevFPDataIndex;
	while (0 <= fpDataIndex) {
		data = &fpDataVector[fpDataIndex];
		fpDataIndex = data->prevFPDataIndex;
		int from0 = (isSwapped) ? data->y : data->x;
		int from1 = (isSwapped) ? data->x : data->y;
		if (from1 - from0 < to1 - to0) {
			if (from1 + 1 < to1) {
				outputOperation(DiffOperation::NotChanged, from0, from1 + 1, to1 - (from1 + 1),
					to1 - (from1 + 1));
			}
			outputOperation(DiffOperation::Inserted, from0, from1, 0, 1);
		} else {
			if (from0 + 1 < to0) {
				outputOperation(DiffOperation::NotChanged, from0 + 1, from1, to0 - (from0 + 1),
					to0 - (from0 + 1));
			}
			outputOperation(DiffOperation::Deleted, from0, from1, 1, 0);
		}

		to0 = from0;
		to1 = from1;
	}
	if (to0 != 0)
		outputOperation(DiffOperation::NotChanged, 0, 0, to0, to0);
}


void
NPDiff::outputOperation(DiffOperation::Operator op, int from0, int from1, int count0, int count1)
{
	int diffResultSize = diffResult.size();
	if (0 < diffResultSize) {
		bool toBind = false;
		DiffOperation& lastOperation = diffResult[diffResultSize - 1];
		if (lastOperation.from0 == from0 + count0 && lastOperation.from1 == from1 + count1) {
			if (DiffOperation::Inserted == op) {
				if (DiffOperation::Inserted == lastOperation.op
					|| DiffOperation::Modified == lastOperation.op) {
					toBind = true;
				} else if (DiffOperation::Deleted == lastOperation.op) {
					lastOperation.op = DiffOperation::Modified;
					toBind = true;
				}
			} else if (DiffOperation::Deleted == op) {
				if (DiffOperation::Deleted == lastOperation.op
					|| DiffOperation::Modified == lastOperation.op) {
					toBind = true;
				} else if (DiffOperation::Inserted == lastOperation.op) {
					lastOperation.op = DiffOperation::Modified;
					toBind = true;
				}
			}
		}
		if (toBind) {
			lastOperation.from0 = from0;
			lastOperation.from1 = from1;
			lastOperation.count0 += count0;
			lastOperation.count1 += count1;
			return;
		}
	}

	DiffOperation operation;
	operation.op = op;
	operation.from0 = from0;
	operation.from1 = from1;
	operation.count0 = count0;
	operation.count1 = count1;
	diffResult.push_back(operation);
}


const DiffOperation*
NPDiff::GetOperationAt(int index)
{
	int resultIndex = diffResult.size() - 1 - index;
	if (resultIndex < 0 || index < 0)
		return NULL;

	return &diffResult[resultIndex];
}


int
NPDiff::getLength(int seqNo) const
{
	if (isSwapped)
		seqNo = (seqNo == 0) ? 1 : 0;
	return sequences->GetLength(seqNo);
}


bool
NPDiff::isEqual(int index0, int index1) const
{
	if (isSwapped)
		return sequences->IsEqual(index1, index0);
	else
		return sequences->IsEqual(index0, index1);
}
