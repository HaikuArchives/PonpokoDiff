/*
 * Copyright 2007, ICHIMIYA Hironori (Hiron)
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		ICHIMIYA Hironori (Hiron)
 *
 */
#ifndef NPDIFF_H__INCLUDED
#define NPDIFF_H__INCLUDED

#include <vector>


class Sequences {
public:
						Sequences() {}
	virtual				~Sequences() {}

	virtual	int			GetLength(int seqNo) const = 0;
	virtual bool		IsEqual(int index0, int index1) const = 0;
};


struct DiffOperation {
	enum Operator {
		Inserted = 0,
		Modified,
		Deleted,
		NotChanged,
	};

	Operator	op;
	int			from0;
	int			from1;
	int			count0;
	int			count1;
};

/*
 *	S Wu, U Manber, G Myers, W Miller:
 *  "An O(NP) Sequence Comparison Algorithm",
 *	Information Processing Letters (1990)
 */
class NPDiff {
public:
						NPDiff();
	virtual				~NPDiff();

			void		Detect(const Sequences* sequences);
	const DiffOperation*	GetOperationAt(int index);

private:
			void		snake(int k);
			void		makeResult();
			void		outputOperation(DiffOperation::Operator op,
							int from0, int from1, int count0, int count1);

			int			getLength(int seqNo) const;
			bool		isEqual(int index0, int index1) const;

private:
	// furthest point
	struct FPData {
		int		y;					//< furthest point
		int		x;					//< furthest point
		int		prevFPDataIndex;	//< FPData （NPDiff::fpDataVector）
	};

	typedef std::vector<FPData>	FPDataVector;
	typedef std::vector<DiffOperation> DiffOpVector;

	const Sequences*	sequences;
	bool				isSwapped;		//< m <= n, true
	DiffOpVector		diffResult;

	// Detect()
	FPDataVector		fpDataVector;
	int*				fp;
	int*				fpBuffer;
};

#endif // NPDIFF_H__INCLUDED
