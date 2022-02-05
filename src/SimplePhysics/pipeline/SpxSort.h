#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{

template <typename SortData>
void SpxMergeTwoBuffers(
	SortData* d1,
	unsigned int n1,
	SortData* d2,
	unsigned int n2,
	SortData* buff)
{
	unsigned int i = 0;
	unsigned int j = 0;

	// 片方の配列を使い果たすまではキーを比較してバッファに詰める
	while (i < n1 && j < n2)
	{
		if (d1[i].key < d2[j].key)
		{
			buff[i + j] = d1[i];
			++i;
		}
		else {
			buff[i + j] = d2[j];
			++j;
		}
	}

	// 片方の配列が全て無くなってしまった場合は残りをもう片方で全て埋める
	if (i < n1)	 // まだ1つ目の配列に要素が残っている場合
	{
		while (i < n1)
		{
			buff[i + j] = d1[i];
			++i;
		}
	}
	else if (j < n2)  // まだ2つ目の配列に要素が残っている場合
	{
		while (j < n2)
		{
			buff[i + j] = d2[j];
			++j;
		}
	}

	// 元の配列にバッファの中身をコピーする
	for (unsigned int k = 0; k < (n1 + n2); k++)
	{
		d1[k] = buff[k];
	}
}

/**
 * @brief マージソート(テスト済み)
 *
 * @tparam SortData ソートするデータ型(メンバ変数 key を含む)
 * @param d ソートするデータの配列(中身がソートされる)
 * @param buff バッファとして使う配列
 * @param n データの数
 */
template <typename SortData>
void SpxSort(SortData* d, SortData* buff, int n)
{
	int n1 = n >> 1;  // n1を2で割った値
	int n2 = n - n1;
	if (n1 > 1)
	{
		// dの先頭からn1個分に対して分割->ソート
		SpxSort(d, buff, n1);
	}

	if (n2 > 1)
	{
		// dのn1個目からn2個分に対して分割->ソート
		SpxSort(d + n1, buff, n2);
	}
	SpxMergeTwoBuffers(d, n1, d + n1, n2, buff);  // ソート
}

};	// namespace SimplePhysics