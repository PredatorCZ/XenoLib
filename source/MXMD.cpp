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

#include "MXMD_Internal.h"
#include "DRSM.h"
#include "MXMD_V1.h"
#include "MXMD_V3.h"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"
#include "datas/macroLoop.hpp"
#include "datas/MultiThread.hpp"

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
		uncachedTexturesOffset;

	ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE MXMDModel::Ptr Model()
	{ 
		switch (version)
		{
		case MXMDVer1:
			return MXMDModel::Ptr(new MXMDModel_V1_Wrap(reinterpret_cast<MXMDModel_V1 *>(GetMe() + modelsOffset)));

		case MXMDVer3:
			return MXMDModel::Ptr(new MXMDModel_V3_Wrap(reinterpret_cast<MXMDModel_V3 *>(GetMe() + modelsOffset)));

		default:
			return nullptr;
		}
	}

	ES_FORCEINLINE MXMDGeomBuffers::Ptr Buffers()
	{
		switch (version)
		{
		case MXMDVer1:
			return MXMDGeomBuffers::Ptr(new MXMDGeometryHeader_V1_Wrap(reinterpret_cast<MXMDGeometryHeader_V1 *>(GetMe() + vertexBufferOffset)));

		case MXMDVer3:
		{
			if (vertexBufferOffset)
				return MXMDGeomBuffers::Ptr(new MXMDGeometryHeader_V3_Wrap(reinterpret_cast<MXMDGeometryHeader_V3 *>(GetMe() + vertexBufferOffset)));
			else
				return nullptr;
		}

		default:
			return nullptr;
		}
	}

	ES_FORCEINLINE MXMDMaterials::Ptr Materials()
	{
		switch (version)
		{
		case MXMDVer1:
			return MXMDMaterials::Ptr(new MXMDMaterials_V1_Wrap(reinterpret_cast<MXMDMaterialsHeader_V1 *>(GetMe() + materialsOffset)));

		case MXMDVer3:
			return MXMDMaterials::Ptr(new MXMDMaterials_V3_Wrap(reinterpret_cast<MXMDMaterialsHeader_V3 *>(GetMe() + materialsOffset)));

		default:
			return nullptr;
		}
	}
};

struct xbc1QueueInternal
{
	int queue;
	int queueEnd;
	int *offsets;
	char *mainBuffer;
	std::vector<char *> *buffers;

	typedef void return_type;

	xbc1QueueInternal() : queue(0) {}

	return_type RetreiveItem()
	{
		char *resBuffer = mainBuffer + offsets[queue];
		buffers->at(queue) = ExtractXBC(resBuffer);
	}

	operator bool() { return queue < queueEnd; }
	void operator++(int) { queue++; }
	int NumQueues() const { return queueEnd; }
};

template<class _Ty0>
int MXMD::_Load(const _Ty0 *fileName)
{
	BinReader rd(fileName);
	MXMDHeader hdr;
	rd.Read(hdr);

	switch (hdr.magic)
	{
	case ID:
		rd.SwapEndian(false);
		break;
	case ID_BIG:
		rd.SwapEndian(true);
		FByteswapper(hdr.version);
		break;
	default:
		printerror("[MXMD] Invalid header.");
		return 1;
	}

	bool versionFound = false;

	for (const int &v : versions)
		if (v == hdr.version)
		{
			versionFound = true;
			break;
		}

	if (!versionFound)
	{
		printerror("[MXMD] Unsupported version: ", << hdr.version);
		return 2;
	}

	rd.Seek(0);
	const size_t fileSize = rd.GetSize();

	data.masterBuffer = static_cast<char *>(malloc(fileSize));
	rd.ReadBuffer(data.masterBuffer, fileSize);

	if (rd.SwappedEndian())
	{
		data.header->SwapEndian();
		GetModel()->SwapEndian();
		GetGeometry()->SwapEndian();
		GetTextures()->SwapEndian();
		GetMaterials()->SwapEndian();
	}

	UniString<_Ty0> fileNameExternal = fileName;
	fileNameExternal.replace(fileNameExternal.size() - 3, 3, esString("smt"));

	switch (hdr.version)
	{
	case MXMDVer1:
	{
		BinReader res(fileNameExternal);

		if (res.IsValid())
		{
			const size_t _fileSize = res.GetSize();

			MXMDExternalResource_V1 *externalResourcev1 = new MXMDExternalResource_V1;
			externalResourcev1->buffer = static_cast<char *>(malloc(_fileSize));
			res.ReadBuffer(externalResourcev1->buffer, _fileSize);
			externalResource = externalResourcev1;
		}
		else
		{
			printerror("[MXMD] Cannot load external buffer: ", << fileNameExternal.c_str());
		}

		break;
	}
	case MXMDVer3:
	{
		if (!data.header->cachedTexturesOffset)
		{
			MXMDExternalResource_V3 *externalResourcev3 = new MXMDExternalResource_V3;

			if (externalResourcev3->Load(fileNameExternal.c_str()))
				delete externalResourcev3;
			else
				externalResource = externalResourcev3;
		}
		else
		{
			BinReader res(fileNameExternal);

			if (res.IsValid())
			{
				const size_t _fileSize = res.GetSize();

				char *resBuffer = static_cast<char *>(malloc(_fileSize));
				res.ReadBuffer(resBuffer, _fileSize);

				MXMDExternalResource_V31 *externalResourcev31 = new MXMDExternalResource_V31;
				CASMTHeader_V3 *reshdrData = reinterpret_cast<CASMTHeader_V3 *>(data.header->GetMe() + data.header->uncachedTexturesOffset);

				xbc1QueueInternal xbcQue;
				xbcQue.queueEnd = reshdrData->numGroups;
				xbcQue.offsets = reshdrData->GetGroupDataOffsets();
				xbcQue.buffers = &externalResourcev31->buffers;
				xbcQue.mainBuffer = resBuffer;
				externalResourcev31->buffers.resize(xbcQue.queueEnd);

				RunThreadedQueue(xbcQue);

				externalResource = externalResourcev31;
				free(resBuffer);
			}
			else
			{
				printerror("[MXMD] Cannot load external buffer: ", << fileNameExternal.c_str());
			}

		}

		break;
	}
	}
	return 0;
}

template int MXMD::_Load(const char *fileName);
template int MXMD::_Load(const wchar_t *fileName);

MXMDModel::Ptr MXMD::GetModel() { return data.header->Model(); }
MXMDMaterials::Ptr MXMD::GetMaterials() { return data.header->Materials(); }

MXMDGeomBuffers::Ptr MXMD::GetGeometry() 
{ 
	MXMDGeomBuffers::Ptr buff = data.header->Buffers();
	
	if (!buff && data.header->version == MXMDVer3)
	{
		MXMDExternalResource_V3 *res = static_cast<MXMDExternalResource_V3 *>(externalResource);
		return MXMDGeomBuffers::Ptr(new MXMDGeometryHeader_V3_Wrap(reinterpret_cast<MXMDGeometryHeader_V3 *>(res->GetResource(0))));
	}

	return buff;
}

MXMDTextures::Ptr MXMD::GetTextures()
{
	switch (data.header->version)
	{
	case MXMDVer1:
	{
		MXMDExternalResource_V1 *res = static_cast<MXMDExternalResource_V1 *>(externalResource);
		return MXMDTextures::Ptr(new MXMDTextures_V1_Wrap(reinterpret_cast<CASMTHeader *>(
			data.header->uncachedTexturesOffset ? data.header->GetMe() + data.header->uncachedTexturesOffset : nullptr),
			res ? res->buffer : nullptr,
			reinterpret_cast<CASMTGroup *>(data.header->GetMe() + data.header->cachedTexturesOffset)));
	}

	case MXMDVer3:
	{
		if (!data.header->cachedTexturesOffset)
			return MXMDTextures::Ptr(new MXMDTextures_V3_Wrap(static_cast<DRSM *>(static_cast<MXMDExternalResource_V3 *>(externalResource))));
		else
		{
			MXMDExternalResource_V31 *res = static_cast<MXMDExternalResource_V31 *>(externalResource);
			return MXMDTextures::Ptr(new MXMDTextures_V31_Wrap(reinterpret_cast<CASMTHeader_V3 *>(
				data.header->GetMe() + data.header->uncachedTexturesOffset),
				res,
				reinterpret_cast<CASMTGroup *>(data.header->GetMe() + data.header->cachedTexturesOffset)));
		}
	}

	default:
		return nullptr;
	}
}

template<class _Ty>
struct TextureQueue
{
	int queue;
	int queueEnd;
	const MXMDTextures *caller;
	TextureConversionParams params;
	const _Ty *folderPath;

	typedef int return_type;

	TextureQueue() : queue(0) {}

	return_type RetreiveItem()
	{
		int result = caller->ExtractTexture(folderPath, queue, params);
		return result;
	}

	operator bool() { return queue < queueEnd; }
	void operator++(int) { queue++; }
	int NumQueues() const { return queueEnd; }
};


template<class _Ty>
int MXMDTextures::_ExtractAllTextures(const _Ty *outputFolder, TextureConversionParams params) const
{
	TextureQueue<_Ty> texQue;
	texQue.params = params;
	texQue.caller = this;
	texQue.queueEnd = GetNumTextures();
	texQue.folderPath = outputFolder;
	
	RunThreadedQueue(texQue);

	return 0;
}

template int MXMDTextures::_ExtractAllTextures(const char *outputFolder, TextureConversionParams params) const;
template int MXMDTextures::_ExtractAllTextures(const wchar_t *outputFolder, TextureConversionParams params) const;

MXMD::~MXMD()
{
	if (data.masterBuffer)
		free(data.masterBuffer);

	if (externalResource)
		delete externalResource;
}

template<MXMDVertexDescriptorType E> MXMDVertexDescriptor_Internal *_descDummy() { return new tVertexDescriptor<E>(); }

#define _ADD_NEW_DESC(value) {value, _descDummy<value>},

const DescMap _MXMDDescriptorRegistry =
{
	StaticFor(_ADD_NEW_DESC,
	MXMD_POSITION,
	MXMD_WEIGHT32,
	MXMD_BONEID,
	MXMD_WEIGHTID,
	MXMD_UV1,
	MXMD_UV2,
	MXMD_UV3,
	MXMD_NORMAL32,
	MXMD_VERTEXCOLOR,
	MXMD_NORMAL,
	MXMD_NORMAL2,
	MXMD_WEIGHT16,
	MXMD_BONEID2,
	MXMD_NORMALMORPH,
	MXMD_MORPHVERTEXID)
};