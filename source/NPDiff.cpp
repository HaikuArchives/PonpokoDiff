/*
 * PonpokoDiff
 *
 * Copyright (c) 2008 PonpokoDiff Project Contributors
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 *	@file		NPDiff.cpp
 *	@brief		NPDiff クラスの実装
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-15 Created
 */

#include "NPDiff.h"
#include <stdlib.h>
#include <string.h>
#include "Exception.h"
#include "ExceptionCode.h"

/**
 *	@brief	コンストラクタ
 */
NPDiff::NPDiff()
{
	fp = NULL;
	fpBuffer = NULL;
	sequences = NULL;
	isSwapped = false;
}

/**
 *	@brief	デストラクタ
 */
NPDiff::~NPDiff()
{
}

/**
 *	@brief	差分を検出します。
 *
 *	O(NP) アルゴリズムで探索します。
 *	探索結果となる FPData の x, y はそれぞれ
 *	短い符号列のインデックス + 1、長い符号列のインデックス + 1 に
 *	なっていることに注意。
 *
 *	@param	sequences	差分抽出を行う符号列
 */
void NPDiff::Detect(const Sequences* sequences)
{
	this->sequences = sequences;
	isSwapped = (sequences->GetLength(0) > sequences->GetLength(1));
	diffResult.clear();
	
	if (NULL == sequences)
	{
		// 符号列がセットされてない場合は結果をクリアするだけ
		return;
	}
	
	// 前処理
	int m = getLength(0);
	int n = getLength(1);
	int delta = n - m;
	
	fpBuffer = static_cast<int*>(malloc((m + n + 3) * sizeof(int)));
	if (NULL == fpBuffer)
	{
		// メモリが足りない。
		MemoryException::Throw();
	}
	memset(fpBuffer, 0, (m + n + 3) * sizeof(int));
	fp = fpBuffer + m + 1;
	
	// 探索
	int p;
	for (p = 0; p < m; p++)		// p < m は念のため
	{
		int k;
		for (k = -p; k <= delta - 1; k++)
		{
			snake(k);
		}
		for (k = delta + p; k >= delta; k--)
		{
			snake(k);
		}
		
		int fpDataIndexDelta = fp[delta] - 1;
		int fpDelta = (0 > fpDataIndexDelta) ? -1 : fpDataVector[fpDataIndexDelta].y;
		if (fpDelta == n)
		{
			break;
		}
	}
	
	// 探索したパスを復元して差分抽出結果を作る
	makeResult();
	
	// 後処理
	free(fpBuffer);
	fpBuffer = fp = NULL;
	fpDataVector.clear();
}

/**
 *	@brief	snake 処理。
 *	単に diagonal edge をたどる処理（snake 処理）に加え、
 *	たどったパスの記憶も行います。
 */
void NPDiff::snake(int k)
{
	int fpDataIndex0 = fp[k - 1] - 1;
	int fpDataIndex1 = fp[k + 1] - 1;
	
	int fpY0 = (0 > fpDataIndex0) ? -1 : fpDataVector[fpDataIndex0].y;
	int fpY1 = (0 > fpDataIndex1) ? -1 : fpDataVector[fpDataIndex1].y;

	FPData data;
	if (fpY0 + 1 > fpY1)
	{
		data.y = fpY0 + 1;
		data.prevFPDataIndex = fpDataIndex0;
	}
	else
	{
		data.y = fpY1;
		data.prevFPDataIndex = fpDataIndex1;
	}
	data.x = data.y - k;
	
	while (data.x < getLength(0) && data.y < getLength(1) && isEqual(data.x, data.y))
	{
		data.x++;
		data.y++; 
	}
	
	fpDataVector.push_back(data);
	fp[k] = static_cast<int>(fpDataVector.size());
}

/**
 *	@brief	探索のパスをたどって差分抽出結果を求めます。
 */
void NPDiff::makeResult()
{
	// まず、fp[delta] - 1 のインデックスを代入
	// これは最後のスネークで見つかるはずなので
	// 最後に追加された furthest pint 情報のインデックスに等しい
	int fpDataIndex = fpDataVector.size() - 1;
	const FPData* data = &fpDataVector[fpDataIndex];
	int to0 = (isSwapped) ? data->y : data->x;
	int to1 = (isSwapped) ? data->x : data->y;
	fpDataIndex = data->prevFPDataIndex;
	while (0 <= fpDataIndex)
	{
		data = &fpDataVector[fpDataIndex];
		fpDataIndex = data->prevFPDataIndex;
		int from0 = (isSwapped) ? data->y : data->x;
		int from1 = (isSwapped) ? data->x : data->y;
		if (from1 - from0 < to1 - to0)
		{
			// 挿入
			if (from1 + 1 < to1)
			{
				// スネークの分を先に出力
				outputOperation(DiffOperation::NotChanged,
								from0,
								from1 + 1,
								to1 - (from1 + 1),
								to1 - (from1 + 1));
			}
			outputOperation(DiffOperation::Inserted,
							from0,
							from1,
							0,
							1);
		}
		else
		{
			// 削除
			if (from0 + 1 < to0)
			{
				// スネークの分を先に出力
				outputOperation(DiffOperation::NotChanged,
								from0 + 1,
								from1,
								to0 - (from0 + 1),
								to0 - (from0 + 1));
			}
			outputOperation(DiffOperation::Deleted,
							from0,
							from1,
							1,
							0);
		}

		to0 = from0;
		to1 = from1;	
	}
	if (to0 != 0)
	{
		// 最初の operation が変化なしの場合のみここにくる
		// 最初が追加や削除なら、初回の探索で (0,0) が furthest point になるので
		// ループを抜けた時点で to0 == to1 == 0 になっているはずだから。
		outputOperation(DiffOperation::NotChanged,
						0,
						0,
						to0,
						to0);
	}
}

/**
 *	@brief	makeResult() の過程で diff 操作を（後ろから）出力します。
 *	@param[in]	op		操作の種類
 *	@param[in]	from0	符号列 0 のインデックス
 *	@param[in]	from1	符号列 1 のインデックス
 *	@param[in]	count0	符号列 0 がこの操作でどれだけ進むか（操作の符号数）
 *	@param[in]	count1	符号列 1 がこの操作でどれだけ進むか（操作の符号数）
 */
void NPDiff::outputOperation(DiffOperation::Operator op, int from0, int from1, int count0, int count1)
{
	// 連続する削除、連続する挿入は、まとめる。
	// 挿入－(削除または変更)、削除－(挿入または変更)は、変更にまとめる。
	int diffResultSize = diffResult.size();
	if (0 < diffResultSize)
	{
		bool toBind = false;
		DiffOperation& lastOperation = diffResult[diffResultSize - 1];
		if (lastOperation.from0 == from0 + count0 && lastOperation.from1 == from1 + count1)
		{
			if (DiffOperation::Inserted == op)
			{
				if (DiffOperation::Inserted == lastOperation.op ||
					DiffOperation::Modified == lastOperation.op)
				{
					toBind = true;
				}
				else if (DiffOperation::Deleted == lastOperation.op)
				{
					lastOperation.op = DiffOperation::Modified;
					toBind = true;
				}
			}
			else if (DiffOperation::Deleted == op)
			{
				if (DiffOperation::Deleted == lastOperation.op ||
					DiffOperation::Modified == lastOperation.op)
				{
					toBind = true;
				}
				else if (DiffOperation::Inserted == lastOperation.op)
				{
					lastOperation.op = DiffOperation::Modified;
					toBind = true;
				}
			}
		}
		if (toBind)
		{
			lastOperation.from0 = from0;
			lastOperation.from1 = from1;
			lastOperation.count0 += count0;
			lastOperation.count1 += count1;
			return;			
		}
	}
	
	// まとめられなかった場合は追加する
	DiffOperation operation;
	operation.op = op;
	operation.from0 = from0;
	operation.from1 = from1;
	operation.count0 = count0;
	operation.count1 = count1;
	diffResult.push_back(operation);
}

/**
 *	@brief	指定したインデックス番目の差分操作を得ます。
 *	@param[in]	index	インデックス
 *	@return	差分操作が返ります。指定したインデックス番目に操作がない場合は NULL が返ります。
 */
const DiffOperation* NPDiff::GetOperationAt(int index)
{
	// diffResult は逆順に出力されているのでインデックスを逆順に調整する
	int resultIndex = diffResult.size() - 1 - index;
	if (resultIndex < 0 || index < 0)
	{
		return NULL;
	}
	
	return &diffResult[resultIndex];
}

/**
 *	@brief	指定した符号列の長さを得ます。
 *	@param[in]	seqNo	0 または 1 を指定。
 *	@return	符号列の長さ
 */
int NPDiff::getLength(int seqNo) const
{
	if (isSwapped)
	{
		seqNo = (seqNo == 0) ? 1 : 0;
	}
	return sequences->GetLength(seqNo);
}

/**
 *	@brief	指定したインデックスの符号が一致するかどうかを調べます。
 *	@param[in]	index0	符号列 0 のインデックス (0 を起点とします)
 *	@param[in]	index1	符号列 1 のインデックス (0 を起点とします)
 *	@return	一致するなら true。
 */
bool NPDiff::isEqual(int index0, int index1) const
{
	if (isSwapped)
	{
		return sequences->IsEqual(index1, index0);
	}
	else
	{
		return sequences->IsEqual(index0, index1);
	}
}


