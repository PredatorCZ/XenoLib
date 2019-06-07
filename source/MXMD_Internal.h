/*  Xenoblade Engine Format Library
	Copyright(C) 2017-2019 Lukas Cone

	This program is free software : you can redistribute it and / or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include "datas/endian.hpp"
#include "datas/vectors.hpp"
#include "MXMD.h"

template<class E, class C> ES_INLINE void _ArraySwap(C &input)
{
	const size_t numItems = sizeof(C) / sizeof(E);
	E *inputPtr = reinterpret_cast<E *>(&input);

	for (size_t t = 0; t < numItems; t++)
		FByteswapper(*(inputPtr + t));
}

enum MXMDVersions
{
	MXMDVer1 = 10040,
	MXMDVer2 = 10111,
	MXMDVer3 = 10112,
};

static constexpr MXMDVersions versions[] = { MXMDVer1, MXMDVer3 };

struct MXMDHeader
{
	int magic,
		version,
		modelsOffset,
		materialsOffset,
		unkOffset0,
		vertexBufferOffset,
		shadersOffset,
		cachedTexturesOffset,
		unkOffset1,
		uncachedTexturesOffset,
		//custom values
		externalBufferIDsOffset,
		externalBufferIDsCount,
		null00[5];

	ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
};

class MXMDVertexDescriptor_Internal;

struct MXMDVertexType
{
	short type,
		size;

	ES_FORCEINLINE void SwapEndian() { _ArraySwap<short>(*this); }
	MXMDVertexDescriptor_Internal *GetWorker();
};
