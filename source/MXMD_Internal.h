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
#include <map>
#include "datas/endian.hpp"
#include "datas/vectors.hpp"
#include "datas/masterprinter.hpp"
#include "datas/esstring.h"
#include "MXMD.h"

template<class E, class C> ES_INLINE void _ArraySwap(C &input)
{
	const size_t numItems = sizeof(C) / sizeof(E);
	E *inputPtr = reinterpret_cast<E *>(&input);

	for (size_t t = 0; t < numItems; t++)
		FByteswapper(*(inputPtr + t));
}

class MXMDVertexDescriptor_Internal : public MXMDVertexDescriptor
{
public:
	char *buffer;
	int stride;
	int count;
	MXMDVertexDescriptorType type;
	virtual void SwapEndian(int count) {}
	MXMDVertexDescriptorType Type() const { return type; }
	int Size() const { return count; }
};

template<MXMDVertexDescriptorType> class tVertexDescriptor;

template<>class tVertexDescriptor<MXMD_POSITION> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		Vector &out = *static_cast<Vector *>(data);
		out = *reinterpret_cast<Vector *>(buffer + stride * at);
	}

	void SwapEndian(int count)
	{
		for (int v = 0; v < count; v++)
			reinterpret_cast<Vector *>(buffer + stride * v)->SwapEndian();
	}
};

template<>class tVertexDescriptor<MXMD_NORMAL32> : public tVertexDescriptor<MXMD_POSITION> {};
template<>class tVertexDescriptor<MXMD_WEIGHT32> : public tVertexDescriptor<MXMD_POSITION> {};

template<>class tVertexDescriptor<MXMD_NORMAL> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		Vector &out = *static_cast<Vector *>(data);
		out = reinterpret_cast<CVector *>(buffer + stride * at)->Convert<float>() * (1.0f / CHAR_MAX);
	}
};
template<>class tVertexDescriptor<MXMD_NORMAL2> : public tVertexDescriptor<MXMD_NORMAL> {};

template<>class tVertexDescriptor<MXMD_BONEID> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		UCVector4 &out = *static_cast<UCVector4 *>(data);
		out = *reinterpret_cast<UCVector4 *>(buffer + stride * at);
	}
};
template<>class tVertexDescriptor<MXMD_BONEID2> : public tVertexDescriptor<MXMD_BONEID> {};

template<>class tVertexDescriptor<MXMD_WEIGHTID> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		ushort &out = *static_cast<unsigned short *>(data);
		out = *reinterpret_cast<ushort *>(buffer + stride * at);
	}

	void SwapEndian(int count)
	{
		for (int v = 0; v < count; v++)
			FByteswapper(*reinterpret_cast<ushort *>(buffer + stride * v));
	}
};

template<>class tVertexDescriptor<MXMD_VERTEXCOLOR> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		Vector4 &out = *static_cast<Vector4 *>(data);
		out = reinterpret_cast<UCVector4 *>(buffer + stride * at)->Convert<float>() * (1.0f / UCHAR_MAX);
	}
};

template<>class tVertexDescriptor<MXMD_UV1> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		Vector2 &out = *static_cast<Vector2 *>(data);
		out = *reinterpret_cast<Vector2 *>(buffer + stride * at);
	}

	void SwapEndian(int count)
	{
		for (int v = 0; v < count; v++)
			reinterpret_cast<Vector2 *>(buffer + stride * v)->SwapEndian();
	}
};
template<>class tVertexDescriptor<MXMD_UV2> : public tVertexDescriptor<MXMD_UV1> {};
template<>class tVertexDescriptor<MXMD_UV3> : public tVertexDescriptor<MXMD_UV1> {};

template<>class tVertexDescriptor<MXMD_WEIGHT16> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		Vector4 &out = *static_cast<Vector4 *>(data);
		out = reinterpret_cast<USVector4 *>(buffer + stride * at)->Convert<float>() * (1.0f / USHRT_MAX);
	}

	void SwapEndian(int count)
	{
		for (int v = 0; v < count; v++)
			reinterpret_cast<USVector4 *>(buffer + stride * v)->SwapEndian();
	}
};

template<>class tVertexDescriptor<MXMD_NORMALMORPH> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		Vector4 &out = *static_cast<Vector4 *>(data);
		out = (reinterpret_cast<UCVector4 *>(buffer + stride * at)->Convert<float>() - 127.f) * (1.f / SCHAR_MAX);
	}
};

template<>class tVertexDescriptor<MXMD_MORPHVERTEXID> : public MXMDVertexDescriptor_Internal
{
	void Evaluate(int at, void *data)
	{
		int &out = *static_cast<int *>(data);
		out = *reinterpret_cast<int *>(buffer + stride * at);
	}
};

typedef std::map<MXMDVertexDescriptorType, MXMDVertexDescriptor_Internal *(*)()> DescMap;
extern const DescMap _MXMDDescriptorRegistry;

struct MXMDVertexType
{
	short type,
		size;

	ES_FORCEINLINE void SwapEndian() { _ArraySwap<short>(*this); }
	ES_FORCEINLINE MXMDVertexDescriptor_Internal *GetWorker()
	{
		if (_MXMDDescriptorRegistry.count(static_cast<MXMDVertexDescriptorType>(type)))
			return _MXMDDescriptorRegistry.at(static_cast<MXMDVertexDescriptorType>(type))();

		return nullptr;
	}
};