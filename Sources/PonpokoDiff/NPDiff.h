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
 *	@file		NPDiff.h
 *	@brief		NPDiff クラスとそれに付随する型の定義
 *	@author		ICHIMIYA Hironori (Hiron)
 *	@date		2007-12-15 Created
 */

#ifndef NPDIFF_H__INCLUDED
#define NPDIFF_H__INCLUDED

#include <vector>

/**
 *	@brief	差分抽出を行う対象の符号列にアクセスするためのインタフェース
 */
class Sequences
{
public:
						Sequences() {}
	virtual				~Sequences() {}
	
	/**
	 *	@brief	指定した符号列の長さを得ます。
	 *	@param[in]	seqNo	0 または 1 を指定します。
	 *	@return	符号列の長さを返します。
	 */
	virtual	int			GetLength(int seqNo) const = 0;
	
	/**
	 *	@brief	指定したインデックスの符号が一致するかどうかを調べます。
	 *	@param[in]	index0	符号列 0 のインデックス (0 を起点とします)
	 *	@param[in]	index1	符号列 1 のインデックス (0 を起点とします)
	 *	@return	一致するなら true。
	 */
	virtual bool		IsEqual(int index0, int index1) const = 0;
};

/**
 *	@brief	差分抽出結果に含まれる差分操作1つ分の情報を格納する構造体
 */
struct DiffOperation
{
	/// 操作の種類の定義
	enum Operator
	{
		Inserted = 0,	///< (符号列 1 の符号が) 挿入された
		Modified,		///< 変更された
		Deleted,		///< (符号列 0 の符号が) 削除された
		NotChanged,		///< 変化なし
	};
	
	Operator	op;		///< 操作の種類
	int			from0;	///< 符号列 0 のインデックス
	int			from1;	///< 符号列 1 のインデックス
	int			count0;	///< 符号列 0 がこの操作でどれだけ進むか（操作の符号数）
	int			count1;	///< 符号列 1 がこの操作でどれだけ進むか（操作の符号数）
};

/**
 *	@brief	O(NP) アルゴリズムによる差分検出クラス
 *
 *	このクラスは次の論文で示されている差分抽出アルゴリズムを用いて処理を行います。
 *	S Wu, U Manber, G Myers, W Miller:
 *  "An O(NP) Sequence Comparison Algorithm",
 *	Information Processing Letters (1990)
 */
class NPDiff
{
public:
							NPDiff();
	virtual					~NPDiff();
	
	void					Detect(const Sequences* sequences);
	const DiffOperation*	GetOperationAt(int index);

private:
	void					snake(int k);
	void					makeResult();
	void					outputOperation(DiffOperation::Operator op, int from0, int from1, int count0, int count1);

	int						getLength(int seqNo) const;
	bool					isEqual(int index0, int index1) const;
	
private:
	/// furthest point に関する情報
	struct FPData
	{
		int		y;					///< furthest point の y 座標（論文アルゴリズム中の fp[k] に相当）
		int		x;					///< furthest point の x 座標
		int		prevFPDataIndex;	///< スネーク前の点の FPData （NPDiff::fpDataVector メンバのインデックス）
	};
	
	typedef std::vector<FPData>	FPDataVector;
	typedef std::vector<DiffOperation> DiffOpVector;

	const Sequences*		sequences;		///< 差分抽出を行う符号列たち
	bool					isSwapped;		///< m <= n の条件を満たすために入れ替えを行っているなら true
	DiffOpVector			diffResult;		///< 差分抽出結果

	// 以下、Detect() 中にのみ有効な情報	
	FPDataVector			fpDataVector;	///< 生成した FPData を入れておく配列
	int*					fp;				///< diagonal k 上の furthest point 情報（論文アルゴリズム中の fp に相当。ただし、値が fpDataVectorのインデックス + 1 となっている点が異なる。0 は未探査（論文中では -1）を表す）
	int*					fpBuffer;		///< fp のメモリを確保している実体（fp はこれをずらしている）	
};

#endif // NPDIFF_H__INCLUDED
