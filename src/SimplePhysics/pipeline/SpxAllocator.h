#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{
	class SpxAllocator
	{
	public:
		/**
		 * @brief メモリ確保時に呼ばれるメソッド
		 *
		 * @param bytes 確保するメモリのバイト数
		 * @return void* 確保したメモリのポインタ(失敗した場合はNULL)
		 */
		virtual void* allocate(size_t bytes) = 0;

		/**
		 * @brief メモリ解放時に呼ばれるメソッド
		 *
		 * @param p 解放するメモリのポインタ
		 */
		virtual void deallocate(void* p) = 0;
	};
};	// namespace SimplePhysics