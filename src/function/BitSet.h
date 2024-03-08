//BitSet.h 扩展的bitset
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#include <bitset>
using std::bitset;

namespace ns_my_std
{
	template<long N>
	class myBitSet : public bitset<N>
	{
	public:
		using bitset<N>::reset;
		using bitset<N>::set;
		myBitSet& operator=(char const* b)
		{
			reset();
			long len = strlen(b);
			char const* p = b + len - 1;//字符串顺序为高到低
			for (long i = 0; i < N && p >= b; ++i, --p)
			{
				set(i, ('1' == *p));
			}
			return *this;
		}
		myBitSet& operator=(string const& b)
		{
			return operator=(b.c_str());
		}
	};
}
