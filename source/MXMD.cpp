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

#include <cmath>
#include <cstring>
#include <climits>
#include <map>
#include "MXMD_V3.h"
#include "DRSM.h"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"
#include "datas/macroLoop.hpp"
#include "datas/MultiThread.hpp"

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

MXMDVertexDescriptor_Internal *MXMDVertexType::GetWorker()
{
	if (_MXMDDescriptorRegistry.count(static_cast<MXMDVertexDescriptorType>(type)))
		return _MXMDDescriptorRegistry.at(static_cast<MXMDVertexDescriptorType>(type))();

	return nullptr;
}

class MXMDMeshObject_V1_Wrap : public MXMDMeshObject
{
	MXMDMeshObject_V1 *data;
public:
	MXMDMeshObject_V1_Wrap(MXMDMeshObject_V1 *input) : data(input) {}

	int GetMeshFacesID() const { return data->meshFacesID; }
	int GetUVFacesID() const { return data->UVFacesID; }
	int GetBufferID() const { return data->bufferID; }
	int GetMaterialID() const { return data->materialID; }
	int GetSkinDesc() const { return data->skinDescriptor; }
	int GetGibID() const { return data->gibID; }
	int GetLODID() const { return 0; }
};

class MXMDMeshGroup_V1_Wrap : public MXMDMeshGroup
{
	MXMDMeshGroup_V1 *data;
	char *masterBuffer;
public:
	MXMDMeshGroup_V1_Wrap(MXMDMeshGroup_V1 *input, char *inputBuff) : data(input), masterBuffer(inputBuff) {}

	MXMDMeshObject::Ptr GetMeshObject(int id) const { return MXMDMeshObject::Ptr(new MXMDMeshObject_V1_Wrap(data->GetMeshes(masterBuffer) + id)); }
	int GetNumMeshObjects() const { return data->meshesCount; }
};

class MXMDBone_V1_Wrap : public MXMDBone
{
	MXMDBone_V1 *data;
	char *masterBuffer;
public:
	MXMDBone_V1_Wrap(MXMDBone_V1 *input, char *inputBuff) : data(input), masterBuffer(inputBuff) {}

	const char *GetName() const { return data->GetBoneName(masterBuffer); }
	int GetParentID() const { return data->parentID; }
	const MXMDTransformMatrix *GetAbsTransform() const { return &data->absTransform; }
	const MXMDTransformMatrix *GetTransform() const { return &data->transform; }
};

class MXMDModel_V1_Wrap : public MXMDModel
{
	MXMDModel_V1 *data;
	std::vector<MXMDBone_V1 *> remapBones;
public:
	MXMDModel_V1_Wrap(MXMDModel_V1 *input) : data(input)
	{
		if (data->assemblyCount > 0xFFF || !data->nodesOffset || !data->boneListOffset)
			return;

		int *nameOffsets = data->GetBoneNamesOffsets();

		if (!nameOffsets)
			return;

		remapBones.resize(data->boneNamesCount);

		for (int r = 0; r < data->boneNamesCount; r++)
		{
			const char *bneName = reinterpret_cast<const char *>(nameOffsets) + nameOffsets[r];

			for (int b = 0; b < data->nodesCount; b++)
			{
				MXMDBone_V1 *cBone = data->GetBones() + b;

				if (!strcmp(cBone->GetBoneName(data->GetBones()->GetMe()), bneName))
				{
					remapBones[r] = cBone;
					break;
				}
			}
		}

	}

	MXMDMeshGroup::Ptr GetMeshGroup(int id) const { return MXMDMeshGroup::Ptr(new MXMDMeshGroup_V1_Wrap(data->GetMeshGroups() + id, data->GetMe())); }
	int GetNumMeshGroups() const { return data->assemblyCount; }

	MXMDBone::Ptr GetBone(int id) const { return MXMDBone::Ptr(new MXMDBone_V1_Wrap(data->GetBones() + id, data->GetBones()->GetMe())); }
	int GetNumBones() const { return data->nodesCount; }

	MXMDBone::Ptr GetSkinBone(int id) const { return MXMDBone::Ptr(new MXMDBone_V1_Wrap(remapBones[id], data->GetBones()->GetMe())); }
	int GetNumSkinBones() const { return static_cast<int>(remapBones.size()); }

	const char *GetMorphName(int id) const { return nullptr; }

	void SwapEndian() { data->SwapEndian(); }
};

class MXMDVertexBuffer_V1_Wrap : public MXMDVertexBuffer
{
	MXMDVertexBuffer_V1 *data;
	char *masterBuffer;
public:
	MXMDVertexBuffer_V1_Wrap(MXMDVertexBuffer_V1 *input, char *inputBuffer) : data(input), masterBuffer(inputBuffer) {}

	int NumVertices() const { return data->count; }
	DescriptorCollection GetDescriptors() const
	{
		MXMDVertexType *desc = data->Descriptors(masterBuffer);
		DescriptorCollection coll;
		int currentOffset = 0;

		for (int d = 0; d < data->descriptorsCount; d++)
		{
			MXMDVertexDescriptor_Internal *worker = desc[d].GetWorker();

			if (!worker)
			{
				currentOffset += desc[d].size;
				continue;
			}

			worker->buffer = data->Buffer(masterBuffer) + currentOffset;
			worker->stride = data->stride;
			worker->count = data->count;
			worker->type = static_cast<MXMDVertexDescriptorType>(desc[d].type);
			currentOffset += desc[d].size;
			coll.push_back(DescriptorCollection::value_type(worker));
		}

		return coll;
	}
};

class MXMDFaceBuffer_V1_Wrap : public MXMDFaceBuffer
{
	MXMDFaceBuffer_V1 *data;
	char *masterBuffer;
public:
	MXMDFaceBuffer_V1_Wrap(MXMDFaceBuffer_V1 *input, char *inputBuffer) : data(input), masterBuffer(inputBuffer) {}

	int GetNumIndices() const { return data->count; }
	const USVector *GetBuffer() const { return data->Buffer(masterBuffer); }
};

class MXMDGeometryHeader_V1_Wrap : public MXMDGeomBuffers
{
	MXMDGeometryHeader_V1 *data;
public:
	MXMDGeometryHeader_V1_Wrap(MXMDGeometryHeader_V1 *input) : data(input) {}

	MXMDVertexBuffer::Ptr GetVertexBuffer(int id) const { return MXMDVertexBuffer::Ptr(new MXMDVertexBuffer_V1_Wrap(data->GetVertexBuffers() + id, data->GetMe())); }
	int GetNumVertexBuffers() const { return data->vertexBuffersCount; }
	MXMDFaceBuffer::Ptr GetFaceBuffer(int id) const { return MXMDFaceBuffer::Ptr(new MXMDFaceBuffer_V1_Wrap(data->GetFaceBuffers() + id, data->GetMe())); }
	int GetNumFaceBuffers() const { return data->faceBuffersCount; }
	MXMDGeomVertexWeightBuffer::Ptr GetWeightsBuffer(int flags) const;
	MXMDMorphTargets::Ptr GetVertexBufferMorphTargets(int vertexBufferID) const { return nullptr; }
	void SwapEndian();
};

class MXMDGeomVertexWeightBuffer_V1 : public MXMDGeomVertexWeightBuffer
{
public:
	MXMDVertexBuffer::DescriptorCollection wtb1,
		wtb2;

	MXMDVertexWeight GetVertexWeight(int id) const;
};

void MXMDGeometryHeader_V1_Wrap::SwapEndian()
{
	data->SwapEndian();

	const int numVBuffers = GetNumVertexBuffers();

	for (int v = 0; v < numVBuffers; v++)
	{
		MXMDVertexBuffer::Ptr vBuff = GetVertexBuffer(v);
		MXMDVertexBuffer::DescriptorCollection dColl = vBuff->GetDescriptors();

		for (auto &d : dColl)
			static_cast<MXMDVertexDescriptor_Internal &>(*d).SwapEndian(vBuff->NumVertices());
	}
}

class MXMDMaterial_V1_Wrap : public MXMDMaterial
{
	MXMDMaterial_V1 *material;
	char *masterBuffer;
public:
	MXMDMaterial_V1_Wrap(MXMDMaterial_V1 *inMat, char *buffer) : material(inMat), masterBuffer(buffer) {}

	virtual int GetNumTextures() const { return material->texturesCount; }
	virtual int GetTextureIndex(int id) const { return material->GetTextureLinks(masterBuffer)[id].textureID; }
	virtual const char *GetName() const { return material->GetName(masterBuffer); }
};

class MXMDMaterials_V1_Wrap : public MXMDMaterials
{
	MXMDMaterialsHeader_V1 *materials;
public:
	MXMDMaterials_V1_Wrap(MXMDMaterialsHeader_V1 *imats) : materials(imats) {}

	MXMDMaterial::Ptr GetMaterial(int id) const { return MXMDMaterial::Ptr(new MXMDMaterial_V1_Wrap(materials->GetMaterials() + id, materials->GetMe())); }
	int GetNumMaterials() const { return materials->materialsCount; }

	void SwapEndian() { materials->SwapEndian(); }
};

class MXMDExternalResource_V1 : public MXMDExternalResource
{
public:
	char *buffer;
	MXMDExternalResource_V1() : buffer(nullptr) {}
	~MXMDExternalResource_V1()
	{
		if (buffer)
			free(buffer);
	}
};

class MXMDTextures_V1_Wrap : public MXMDTextures
{
	char *buffer;
	CASMTHeader *unchached;
	CASMTGroup *cached;
public:
	MXMDTextures_V1_Wrap(CASMTHeader *_uncached, char *_buffer, CASMTGroup *_cached) :unchached(_uncached), buffer(_buffer), cached(_cached) {}

	int GetNumTextures() const { return cached->count; }
	const char *GetTextureName(int id) const
	{
		return cached->GetMe() + cached->GetTextures()[id].nameOffset;
	}

	int ExtractTexture(const wchar_t *outputFolder, int id, TextureConversionParams params) const { return _ExtractTexture(outputFolder, id, params); }
	int ExtractTexture(const char *outputFolder, int id, TextureConversionParams params) const { return _ExtractTexture(outputFolder, id, params); }

	template<class _Ty> int _ExtractTexture(const _Ty *outputFolder, int id, TextureConversionParams params) const;

	void SwapEndian();
};

MXMDGeomVertexWeightBuffer::Ptr MXMDGeometryHeader_V1_Wrap::GetWeightsBuffer(int flags) const
{
	flags &= 0xff;
	MXMDGeomVertexWeightBuffer_V1 *wbuff = new MXMDGeomVertexWeightBuffer_V1;

	switch (flags)
	{
	case 1:
	{
		if (data->mergeData[4])
		{
			wbuff->wtb1 = GetVertexBuffer(data->mergeData[4])->GetDescriptors();
			wbuff->wtb2 = GetVertexBuffer(data->mergeData[0])->GetDescriptors();
		}
		else
		{
			wbuff->wtb1 = GetVertexBuffer(data->mergeData[0])->GetDescriptors();
			wbuff->wtb2 = GetVertexBuffer(data->mergeData[4])->GetDescriptors();
		}
		break;
	}
	case 2:
	case 64:
		wbuff->wtb1 = GetVertexBuffer(data->mergeData[1])->GetDescriptors();
		break;
	case 8:
		wbuff->wtb1 = GetVertexBuffer(data->mergeData[3])->GetDescriptors();
		wbuff->wtb2 = GetVertexBuffer(data->mergeData[4])->GetDescriptors();
		break;
	case 0x21:
		wbuff->wtb1 = GetVertexBuffer(data->mergeData[4])->GetDescriptors();
		break;
	default:
		delete wbuff;
		return nullptr;
	}

	return MXMDGeomVertexWeightBuffer::Ptr(wbuff);
}

MXMDVertexWeight MXMDGeomVertexWeightBuffer_V1::GetVertexWeight(int at) const
{
	MXMDVertexWeight nw = {};
	const int wtb1Size = wtb1[0]->Size();

	if (wtb2.size() && at >= wtb1Size)
	{
		for (auto &d : wtb2)
		{
			switch (d->Type())
			{
			case MXMD_WEIGHT32:
				d->Evaluate(at - wtb1Size, &nw.weights);
				nw.weights.W = fmaxf(1.f - nw.weights.X - nw.weights.Y - nw.weights.Z, 0.f);
				break;
			case MXMD_BONEID:
				d->Evaluate(at - wtb1Size, &nw.boneids);
				break;
			default:
				break;
			}
		}
	}
	else if (wtb1.size())
	{
		for (auto &d : wtb1)
		{
			switch (d->Type())
			{
			case MXMD_WEIGHT32:
				d->Evaluate(at, &nw.weights);
				nw.weights.W = fmaxf(1.f - nw.weights.X - nw.weights.Y - nw.weights.Z, 0.f);
				break;
			case MXMD_BONEID:
				d->Evaluate(at, &nw.boneids);
				break;
			default:
				break;
			}
		}
	}

	return nw;
}

void MXMDTextures_V1_Wrap::SwapEndian()
{
	if (unchached)
		unchached->SwapEndian();

	if (cached)
		cached->SwapEndian();
}

template<class _Ty>
int MXMDTextures_V1_Wrap::_ExtractTexture(const _Ty *outputFolder, int id, TextureConversionParams params) const
{
	CASMTTexture *foundTexture = nullptr;
	const char *textureName = nullptr;
	const char *textureData = nullptr;

	if (buffer && unchached)
		for (int g = unchached->numGroups - 1; g > -1 && !foundTexture; g--)
		{
			short *ids = unchached->GetGroupTextureIDs(g);
			CASMTGroup *grp = unchached->GetGroup(g);

			for (int i = 0; i < grp->count; i++)
				if (ids[i] == id)
				{
					foundTexture = grp->GetTextures() + i;
					textureData = buffer + unchached->GetGroupDataOffsets()[g] + foundTexture->offset;
					textureName = grp->GetMe() + foundTexture->nameOffset;
					break;
				}
		}

	if (!foundTexture && cached)
	{
		foundTexture = cached->GetTextures() + id;
		textureName = cached->GetMe() + foundTexture->nameOffset;
		textureData = cached->GetMe() + foundTexture->offset;
	}

	if (!foundTexture)
	{
		printerror("[MXMD] Cannot find texture: ", << id);
		return 1;
	}

	const UniString<_Ty> tex = outputFolder + esStringConvert<_Ty>(textureName);

	if (ConvertMTXT(textureData, foundTexture->size, tex.c_str(), params))
	{
		printerror("[MXMD] Texture: ", << id << " could not be created");
		return 2;
	}

	return 0;
}


class MXMDInstances_V1_Wrap : public MXMDInstances
{
	MXMDInstancesHeader_V1 *data;
public:
	MXMDInstances_V1_Wrap(MXMDInstancesHeader_V1 *input) : data(input) {}

	int GetNumInstances() const { return data->matricesCount; }
	const MXMDTransformMatrix *GetTransform(int id) const { return &data->GetMatrices()[id].mtx; }
	int GetStartingGroup(int id) const { return data->GetLookups()[data->GetMatrices()[id].lookupIndex].groupIDStart; }
	int GetNumGroups(int id) const { return data->GetLookups()[data->GetMatrices()[id].lookupIndex].numGroups; }
	int GetMeshGroup(int id) const { return data->GetInstanceItems()[id].meshGroup; }
	void SwapEndian() { data->SwapEndian(); }
};

class MXMDShaders_V1_Wrap : public MXMDShaders
{
	MXMDShadersHeader_V1 *data;
public:
	MXMDShaders_V1_Wrap(MXMDShadersHeader_V1 *input) : data(input) {}

	int GetNumShaders() const { return data->numShaders; }
	void *GetShaderFile(int id) const { return data->GetMe() + data->GetShaderItems()[id].offset; }

	void SwapEndian() { data->SwapEndian(); }
};

class MXMDExternalTextures_V1_Wrap : public MXMDExternalTextures
{
	MXMDExternalTexture_V1 *data;
	int numItems;
public:
	MXMDExternalTextures_V1_Wrap(MXMDExternalTexture_V1 *input, int numTexs) : data(input), numItems(numTexs) {}

	int GetNumTextures() const { return numItems; }
	int GetExTextureID(int id) const 
	{
		if (data[id].externalTextureID < 0)
			return data[id].textureID;
		else
			return data[id].externalTextureID;
	}

	int GetContainerID(int id) const
	{
		if (data[id].externalTextureID < 0)
			return data[id].containerID;
		else
			return -1;
	}

	void SwapEndian() 
	{
		for (int t = 0; t < numItems; t++)
			data[t].SwapEndian(); 
	}
};

class MXMDMeshObject_V3_Wrap : public MXMDMeshObject
{
	MXMDMeshObject_V3 *data;
public:
	MXMDMeshObject_V3_Wrap(MXMDMeshObject_V3 *input) : data(input) {}

	int GetMeshFacesID() const { return data->meshFacesID; }
	int GetUVFacesID() const { return data->UVFacesID; }
	int GetBufferID() const { return data->bufferID; }
	int GetMaterialID() const { return data->materialID; }
	int GetSkinDesc() const { return data->skinDescriptor; }
	int GetGibID() const { return 0; }
	int GetLODID() const { return data->LOD; }
};

class MXMDMeshGroup_V3_Wrap : public MXMDMeshGroup
{
	MXMDMeshGroup_V3 *data;
	char *masterBuffer;
public:
	MXMDMeshGroup_V3_Wrap(MXMDMeshGroup_V3 *input, char *inputBuff) : data(input), masterBuffer(inputBuff) {}

	MXMDMeshObject::Ptr GetMeshObject(int id) const { return MXMDMeshObject::Ptr(new MXMDMeshObject_V3_Wrap(data->GetMeshes(masterBuffer) + id)); }
	int GetNumMeshObjects() const { return data->meshesCount; }
};

class MXMDBone_V3_Wrap : public MXMDBone
{
	MXMDBone_V3 *data;
	MXMDTransformMatrix *transform;
	char *masterBuffer;
public:
	MXMDBone_V3_Wrap(MXMDBone_V3 *input, char *inputBuff, MXMDTransformMatrix *inTransforms) : data(input), masterBuffer(inputBuff), transform(inTransforms + data->ID) {}

	const char *GetName() const { return data->GetBoneName(masterBuffer); }
	int GetParentID() const { return -1; }
	const MXMDTransformMatrix *GetAbsTransform() const { return transform; }
	const MXMDTransformMatrix *GetTransform() const { return nullptr; }
};

class MXMDModel_V3_Wrap : public MXMDModel
{
	MXMDModel_V3 *data;
	MXMDSkinBones_V3 *skinBones;
public:
	MXMDModel_V3_Wrap(MXMDModel_V3 *input) : data(input)
	{
		skinBones = data->GetSkinBones();
	}

	MXMDMeshGroup::Ptr GetMeshGroup(int id) const { return MXMDMeshGroup::Ptr(new MXMDMeshGroup_V3_Wrap(data->GetMeshGroups() + id, data->GetMe())); }
	int GetNumMeshGroups() const { return 1; }

	MXMDBone::Ptr GetBone(int id) const { return MXMDBone::Ptr(new MXMDBone_V3_Wrap(skinBones->GetBones() + id, skinBones->GetMe(), skinBones->GetTransforms())); }
	int GetNumBones() const { return skinBones->count1; }

	MXMDBone::Ptr GetSkinBone(int id) const { return GetBone(id); }
	int GetNumSkinBones() const { return GetNumBones(); }

	const char *GetMorphName(int id) const
	{
		MXMDMorphControls_V3 *ctrl = data->GetMorphControls();

		if (!ctrl)
			return nullptr;

		return ctrl->GetControls()[id].GetName1(ctrl->GetMe());
	}
};

class MXMDVertexBuffer_V3_Wrap : public MXMDVertexBuffer
{
	MXMDVertexBuffer_V3 *data;
	char *masterBuffer;
	char *buffer;
public:
	MXMDVertexBuffer_V3_Wrap(MXMDVertexBuffer_V3 *input, char *inputBuffer, char *_buffer) : data(input), masterBuffer(inputBuffer), buffer(_buffer) {}

	int NumVertices() const { return data->count; }
	DescriptorCollection GetDescriptors() const
	{
		MXMDVertexType *desc = data->Descriptors(masterBuffer);
		DescriptorCollection coll;
		int currentOffset = 0;

		for (int d = 0; d < data->descriptorsCount; d++)
		{
			MXMDVertexDescriptor_Internal *worker = desc[d].GetWorker();

			if (!worker)
			{
				currentOffset += desc[d].size;
				continue;
			}

			worker->buffer = data->Buffer(buffer) + currentOffset;
			worker->stride = data->stride;
			worker->count = data->count;
			worker->type = static_cast<MXMDVertexDescriptorType>(desc[d].type);
			currentOffset += desc[d].size;
			coll.push_back(DescriptorCollection::value_type(worker));
		}

		return coll;
	}
};

class MXMDFaceBuffer_V3_Wrap : public MXMDFaceBuffer
{
	MXMDFaceBuffer_V3 *data;
	char *masterBuffer;
public:
	MXMDFaceBuffer_V3_Wrap(MXMDFaceBuffer_V3 *input, char *inputBuffer) : data(input), masterBuffer(inputBuffer) {}

	int GetNumIndices() const { return data->count; }
	const USVector *GetBuffer() const { return data->Buffer(masterBuffer); }
};

class MXMDMorphTargets_V3_Wrap : public MXMDMorphTargets
{
	MXMDMorphDescriptor_V3 *data;
	char *masterBuffer;
	char *vertexBuffer;
	MXMDMorphBuffer_V3 *buffers;
public:
	MXMDMorphTargets_V3_Wrap(MXMDMorphDescriptor_V3 *_data, char *_masterBuffer, MXMDMorphBuffer_V3 *_buffers, char *_buffer) : data(_data), masterBuffer(_masterBuffer), buffers(_buffers), vertexBuffer(_buffer) {}

	int GetNumMorphs() const { return data->targetCount; }
	int GetMorphNameID(int id) const { return data->GetTargetNameIDs(masterBuffer)[id]; }
	MXMDVertexBuffer::DescriptorCollection GetMorph(int id) const
	{
		MXMDMorphBuffer_V3 *buffer = buffers + data->targetOffset + id;
		MXMDVertexBuffer::DescriptorCollection coll;

		if (buffer->type == 4)
		{
			MXMDVertexDescriptor_Internal *index = _MXMDDescriptorRegistry.at(MXMD_MORPHVERTEXID)();

			index->buffer = vertexBuffer + buffer->offset + 28;
			index->stride = buffer->blocksize;
			index->count = buffer->count;
			index->type = MXMD_MORPHVERTEXID;

			coll.push_back(MXMDVertexBuffer::DescriptorCollection::value_type(index));
		}

		MXMDVertexDescriptor_Internal *pos = _MXMDDescriptorRegistry.at(MXMD_POSITION)();

		pos->buffer = vertexBuffer + buffer->offset;
		pos->stride = buffer->blocksize;
		pos->count = buffer->count;
		pos->type = MXMD_POSITION;

		MXMDVertexDescriptor_Internal *norm = _MXMDDescriptorRegistry.at(MXMD_NORMALMORPH)();

		norm->buffer = vertexBuffer + buffer->offset + (buffer->type == 4 ? 16 : 12);
		norm->stride = buffer->blocksize;
		norm->count = buffer->count;
		norm->type = MXMD_NORMALMORPH;

		coll.push_back(MXMDVertexBuffer::DescriptorCollection::value_type(pos));
		coll.push_back(MXMDVertexBuffer::DescriptorCollection::value_type(norm));

		return coll;
	}

	MXMDVertexBuffer::DescriptorCollection GetDeltaMorph(int id) const { return GetMorph(id + 2); }
	MXMDVertexBuffer::DescriptorCollection GetBaseMorph() const { return GetMorph(0); }
};

class MXMDGeometryHeader_V3_Wrap : public MXMDGeomBuffers
{
	MXMDGeometryHeader_V3 *data;
	MXMDMorphsHeader_V3 *morphData;
public:
	MXMDGeometryHeader_V3_Wrap(MXMDGeometryHeader_V3 *input) : data(input), morphData(nullptr)
	{
		if (data->GetBufferManager())
			morphData = data->GetBufferManager()->GetMorphHeader(data->GetMe(), data->GetMe() + data->voxelizedModelOffset);
	}

	MXMDVertexBuffer::Ptr GetVertexBuffer(int id) const { return MXMDVertexBuffer::Ptr(new MXMDVertexBuffer_V3_Wrap(data->GetVertexBuffers() + id, data->GetMe(), data->GetMe() + data->bufferOffset)); }
	int GetNumVertexBuffers() const { return data->vertexBuffersCount; }
	MXMDFaceBuffer::Ptr GetFaceBuffer(int id) const { return MXMDFaceBuffer::Ptr(new MXMDFaceBuffer_V3_Wrap(data->GetFaceBuffers() + id, data->GetMe() + data->bufferOffset)); }
	int GetNumFaceBuffers() const { return data->faceBuffersCount; }
	MXMDGeomVertexWeightBuffer::Ptr GetWeightsBuffer(int flags) const;
	MXMDMorphTargets::Ptr GetVertexBufferMorphTargets(int vertexBufferID) const;
};

class MXMDGeomVertexWeightBuffer_V3 : public MXMDGeomVertexWeightBuffer
{
public:
	MXMDVertexBuffer::DescriptorCollection wtb;
	int bufferOffset;

	MXMDVertexWeight GetVertexWeight(int id) const;
};

class MXMDExternalResource_V31 : public MXMDExternalResource
{
public:
	std::vector<char *>buffers;
	~MXMDExternalResource_V31()
	{
		for (auto &b : buffers)
			free(b);
	}
};

class MXMDTextures_V31_Wrap : public MXMDTextures
{
	MXMDExternalResource_V31 *buffers;
	CASMTHeader_V3 *unchached;
	CASMTGroup *cached;
public:
	MXMDTextures_V31_Wrap(CASMTHeader_V3 *_uncached, MXMDExternalResource_V31 *_buffer, CASMTGroup *_cached) :unchached(_uncached), buffers(_buffer), cached(_cached) {}

	int GetNumTextures() const { return cached->count; }
	const char *GetTextureName(int id) const
	{
		return cached->GetMe() + cached->GetTextures()[id].nameOffset;
	}
	int ExtractTexture(const wchar_t *outputFolder, int id, TextureConversionParams params) const { return _ExtractTexture(outputFolder, id, params); }
	int ExtractTexture(const char *outputFolder, int id, TextureConversionParams params) const { return _ExtractTexture(outputFolder, id, params); }

	template<class _Ty> int _ExtractTexture(const _Ty *outputFolder, int id, TextureConversionParams params) const;
};

class MXMDExternalResource_V3 : public MXMDExternalResource, public DRSM {};

class MXMDTextures_V3_Wrap : public MXMDTextures
{
	DRSM *drsm;
public:
	MXMDTextures_V3_Wrap(DRSM *_drsm) : drsm(_drsm) {}

	int GetNumTextures() const { return drsm->GetNumTextures(); }
	const char *GetTextureName(int id) const { return drsm->GetTextureName(id); }
	int ExtractTexture(const wchar_t *outputFolder, int id, TextureConversionParams params) const { return drsm->ExtractTexture(outputFolder, id, params); }
	int ExtractTexture(const char *outputFolder, int id, TextureConversionParams params) const { return drsm->ExtractTexture(outputFolder, id, params); }
};

class MXMDMaterial_V3_Wrap : public MXMDMaterial
{
	MXMDMaterial_V3 *material;
	char *masterBuffer;
public:
	MXMDMaterial_V3_Wrap(MXMDMaterial_V3 *inMat, char *buffer) : material(inMat), masterBuffer(buffer) {}

	virtual int GetNumTextures() const { return material->texturesCount; }
	virtual int GetTextureIndex(int id) const { return material->GetTextureLinks(masterBuffer)[id].textureID; }
	virtual const char *GetName() const { return material->GetName(masterBuffer); }
};

class MXMDMaterials_V3_Wrap : public MXMDMaterials
{
	MXMDMaterialsHeader_V3 *materials;
public:
	MXMDMaterials_V3_Wrap(MXMDMaterialsHeader_V3 *imats) : materials(imats) {}

	MXMDMaterial::Ptr GetMaterial(int id) const { return MXMDMaterial::Ptr(new MXMDMaterial_V3_Wrap(materials->GetMaterials() + id, materials->GetMe())); }
	int GetNumMaterials() const { return materials->materialsCount; }

	void SwapEndian() { materials->SwapEndian(); }
};

MXMDGeomVertexWeightBuffer::Ptr MXMDGeometryHeader_V3_Wrap::GetWeightsBuffer(int flags) const
{
	char iflags = static_cast<char>(flags);
	char LOD = static_cast<char>((flags & 0xff00) >> 8) - 1;

	if (LOD < 0)
		LOD = 0;

	MXMDBBufferManager_V3 *buffMan = data->GetBufferManager();

	if (!buffMan || !buffMan->numWeightPalettes)
		return nullptr;

	MXMDWeightPalette_V3 *wpal = buffMan->GetWeightPalettes(data->GetMe());

	MXMDGeomVertexWeightBuffer_V3 *wbuff = new MXMDGeomVertexWeightBuffer_V3;
	wbuff->wtb = GetVertexBuffer(buffMan->weightBufferID)->GetDescriptors();
	wbuff->bufferOffset = 0;

	for (int w = 0; w < buffMan->numWeightPalettes; w++)
		if (wpal[w].lod == LOD)
		{
			wbuff->bufferOffset = wpal[w].offsetID - wpal[w].baseOffset;

			if (iflags == 1)
				break;
		}

	return MXMDGeomVertexWeightBuffer::Ptr(wbuff);
}

MXMDVertexWeight MXMDGeomVertexWeightBuffer_V3::GetVertexWeight(int at) const
{
	MXMDVertexWeight nw = {};

	for (auto &d : wtb)
	{
		switch (d->Type())
		{
		case MXMD_WEIGHT16:
			d->Evaluate(at + bufferOffset, &nw.weights);
			nw.weights.W = fmaxf(1.f - nw.weights.X - nw.weights.Y - nw.weights.Z, 0.f);
			break;
		case MXMD_BONEID2:
			d->Evaluate(at + bufferOffset, &nw.boneids);
			break;
		default:
			break;
		}
	}

	return nw;
}

MXMDMorphTargets::Ptr MXMDGeometryHeader_V3_Wrap::GetVertexBufferMorphTargets(int vertexBufferID) const
{
	if (!morphData)
		return nullptr;

	for (int d = 0; d < morphData->descCount; d++)
	{
		MXMDMorphDescriptor_V3 *cDesc = morphData->GetDescriptors(data->GetMe()) + d;

		if (cDesc->bufferID == vertexBufferID)
			return MXMDMorphTargets::Ptr(new MXMDMorphTargets_V3_Wrap(cDesc, data->GetMe(), morphData->GetBufferHeaders(data->GetMe()), data->GetMe() + data->bufferOffset));
	}

	return nullptr;
}

template<class _Ty>
int MXMDTextures_V31_Wrap::_ExtractTexture(const _Ty *outputFolder, int id, TextureConversionParams params) const
{
	CASMTTexture *foundTexture = nullptr;
	const char *textureName = nullptr;
	const char *textureData = nullptr;

	if (unchached && buffers)
		for (int g = unchached->numGroups - 1; g > -1 && !foundTexture; g--)
		{
			short *ids = unchached->GetGroupTextureIDs(g);
			CASMTGroup *grp = unchached->GetGroup(g);

			for (int i = 0; i < grp->count; i++)
				if (ids[i] == id)
				{
					foundTexture = grp->GetTextures() + i;
					textureData = buffers->buffers[g] + foundTexture->offset;
					textureName = grp->GetMe() + foundTexture->nameOffset;
					break;
				}
		}

	if (!foundTexture && cached)
	{
		foundTexture = cached->GetTextures() + id;
		textureName = cached->GetMe() + foundTexture->nameOffset;
		textureData = cached->GetMe() + foundTexture->offset;
	}

	if (!foundTexture)
	{
		printerror("[LBIM] Cannot find texture: ", << id);
		return 1;
	}

	const UniString<_Ty> tex = outputFolder + esStringConvert<_Ty>(textureName);

	if (ConvertLBIM(textureData, foundTexture->size, tex.c_str(), params))
	{
		printerror("[LBIM] Texture: ", << id << " could not be created");
		return 2;
	}

	return 0;
}

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
int MXMD::_Load(const _Ty0 *fileName, bool suppressErrors)
{
	BinReader rd(fileName);

	if (!rd.IsValid())
	{
		if (!suppressErrors)
		{	
			printerror("[MXMD] Cannot open file.");
		}

		return 1;
	}

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
	{
		if (!suppressErrors)
		{	
			printerror("[MXMD] Invalid header.");
		}

		return 2;
	}
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
		if (!suppressErrors)
		{	
			printerror("[MXMD] Unsupported version: ", << hdr.version);
		}

		return 3;
	}

	rd.Seek(0);
	const size_t fileSize = rd.GetSize();

	data.masterBuffer = static_cast<char *>(malloc(fileSize));
	rd.ReadBuffer(data.masterBuffer, fileSize);

	if (rd.SwappedEndian())
	{
		data.header->SwapEndian();
		GetModel()->SwapEndian();

		MXMDGeomBuffers::Ptr geom = GetGeometry();

		if (geom)
			geom->SwapEndian();

		MXMDTextures::Ptr textures = GetTextures();

		if (textures)
			textures->SwapEndian();

		MXMDInstances::Ptr instances = GetInstances();

		if (instances)
			instances->SwapEndian();

		MXMDShaders::Ptr shaders = GetShaders();

		if (shaders)
			shaders->SwapEndian();

		MXMDExternalTextures::Ptr extexts = GetExternalTextures();

		if (extexts)
			extexts->SwapEndian();

		GetMaterials()->SwapEndian();

		if (data.header->externalBufferIDsOffset)
		{
			if (data.header->externalBufferIDsCount < 0)
				reinterpret_cast<MXMDTerrainBufferLookupHeader_V1 *>(data.masterBuffer + data.header->externalBufferIDsOffset)->SwapEndian();
			else
			{
				int *indices = reinterpret_cast<int *>(data.masterBuffer + data.header->externalBufferIDsOffset);

				for (int i = 0; i < data.header->externalBufferIDsCount; i++)
					FByteswapper(indices[i]);
			}
		}
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

			if (data.header->externalBufferIDsOffset)
			{
				std::vector<int> flippedOffsets;

				if (data.header->externalBufferIDsCount < 0)
				{
					MXMDTerrainBufferLookupHeader_V1 *lookups = reinterpret_cast<MXMDTerrainBufferLookupHeader_V1 *>(data.masterBuffer + data.header->externalBufferIDsOffset);
					MXMDTerrainBufferLookup_V1 *bufferLookups = lookups->GetBufferLookups();
					int curBuff = 0;

					for (int i = 0; i < lookups->bufferLookupCount; i++)
						for (int s = 0; s < 2; s++, curBuff++)
						{
							const int &cIndex = bufferLookups[i].bufferIndex[s];
							bool found = false;

							for (auto &o : flippedOffsets)
								if (o == cIndex)
								{
									found = true;
									break;
								}

							if (found)
								continue;

							flippedOffsets.push_back(cIndex);

							MXMDGeometryHeader_V1_Wrap(reinterpret_cast<MXMDGeometryHeader_V1 *>(externalResourcev1->buffer + bufferLookups[i].bufferIndex[s])).SwapEndian();
						}
				}
				else
				{
					int *indices = reinterpret_cast<int *>(data.masterBuffer + data.header->externalBufferIDsOffset);

					for (int i = 0; i < data.header->externalBufferIDsCount; i++)
					{
						const int &cIndex = indices[i];
						bool found = false;

						for (auto &o : flippedOffsets)
							if (o == cIndex)
							{
								found = true;
								break;
							}

						if (found)
							continue;

						flippedOffsets.push_back(cIndex);

						MXMDGeomBuffers::Ptr geom = GetGeometry(i);

						if (!geom)
							continue;

						geom->SwapEndian();
					}
				}
			}
		}
		else if (hdr.uncachedTexturesOffset  || hdr.externalBufferIDsOffset)
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

template int MXMD::_Load(const char *fileName, bool suppressErrors);
template int MXMD::_Load(const wchar_t *fileName, bool suppressErrors);

MXMDModel::Ptr MXMD::GetModel()
{ 
	switch (data.header->version)
	{
	case MXMDVer1:
		return MXMDModel::Ptr(new MXMDModel_V1_Wrap(reinterpret_cast<MXMDModel_V1 *>(data.masterBuffer + data.header->modelsOffset)));

	case MXMDVer3:
		return MXMDModel::Ptr(new MXMDModel_V3_Wrap(reinterpret_cast<MXMDModel_V3 *>(data.masterBuffer + data.header->modelsOffset)));

	default:
		return nullptr;
	}
}
MXMDMaterials::Ptr MXMD::GetMaterials()
{
	switch (data.header->version)
	{
	case MXMDVer1:
		return MXMDMaterials::Ptr(new MXMDMaterials_V1_Wrap(reinterpret_cast<MXMDMaterialsHeader_V1 *>(data.masterBuffer + data.header->materialsOffset)));

	case MXMDVer3:
		return MXMDMaterials::Ptr(new MXMDMaterials_V3_Wrap(reinterpret_cast<MXMDMaterialsHeader_V3 *>(data.masterBuffer + data.header->materialsOffset)));

	default:
		return nullptr;
	}
}

MXMDGeomBuffers::Ptr MXMD::GetGeometry(int groupID) 
{ 
	switch (data.header->version)
	{
	case MXMDVer1:
	{
		if (data.header->vertexBufferOffset)
			return MXMDGeomBuffers::Ptr
			(
				new MXMDGeometryHeader_V1_Wrap
				(
					reinterpret_cast<MXMDGeometryHeader_V1 *>(data.masterBuffer + data.header->vertexBufferOffset)
				)
			);
		else if (data.header->externalBufferIDsOffset)
		{
			MXMDExternalResource_V1 *res = static_cast<MXMDExternalResource_V1 *>(externalResource);
			if (res)
			{
				if (data.header->externalBufferIDsCount < 0)
				{
					MXMDTerrainBufferLookupHeader_V1 *lookups = reinterpret_cast<MXMDTerrainBufferLookupHeader_V1 *>(data.masterBuffer + data.header->externalBufferIDsOffset);
					MXMDTerrainBufferLookup_V1 *bufferLookups = lookups->GetBufferLookups();
					ushort *indices = lookups->GetGroupIndices();

					int outerIndex = indices[groupID];
					int innerIndex = 0;
					
					if (outerIndex >= lookups->bufferLookupCount)
					{
						outerIndex -= lookups->bufferLookupCount;
						innerIndex = 1;
					}

					return MXMDGeomBuffers::Ptr
					(
						new MXMDGeometryHeader_V1_Wrap
						(
							reinterpret_cast<MXMDGeometryHeader_V1 *>(res->buffer + bufferLookups[outerIndex].bufferIndex[innerIndex])
						)
					);
				}
				else
				{
					int *indices = reinterpret_cast<int *>(data.masterBuffer + data.header->externalBufferIDsOffset);
					return MXMDGeomBuffers::Ptr
					(
						new MXMDGeometryHeader_V1_Wrap
						(
							reinterpret_cast<MXMDGeometryHeader_V1 *>(res->buffer + indices[groupID])
						)
					);
				}
			}
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	case MXMDVer3:
	{
		if (data.header->vertexBufferOffset)
			return MXMDGeomBuffers::Ptr
			(
				new MXMDGeometryHeader_V3_Wrap
				(
					reinterpret_cast<MXMDGeometryHeader_V3 *>(data.masterBuffer + data.header->vertexBufferOffset)
				)
			);
		else
		{
			MXMDExternalResource_V3 *res = static_cast<MXMDExternalResource_V3 *>(externalResource);
			return MXMDGeomBuffers::Ptr(new MXMDGeometryHeader_V3_Wrap(reinterpret_cast<MXMDGeometryHeader_V3 *>(res->GetResource(0))));
		}
	}

	default:
		return nullptr;
	}
}

MXMDTextures::Ptr MXMD::GetTextures()
{
	switch (data.header->version)
	{
	case MXMDVer1:
	{
		if (!data.header->cachedTexturesOffset)
			return nullptr;

		MXMDExternalResource_V1 *res = static_cast<MXMDExternalResource_V1 *>(externalResource);
		return MXMDTextures::Ptr(new MXMDTextures_V1_Wrap(reinterpret_cast<CASMTHeader *>(
			data.header->uncachedTexturesOffset ? data.masterBuffer + data.header->uncachedTexturesOffset : nullptr),
			res ? res->buffer : nullptr,
			reinterpret_cast<CASMTGroup *>(data.masterBuffer + data.header->cachedTexturesOffset)));
	}

	case MXMDVer3:
	{
		if (!data.header->cachedTexturesOffset)
			return MXMDTextures::Ptr(new MXMDTextures_V3_Wrap(static_cast<DRSM *>(static_cast<MXMDExternalResource_V3 *>(externalResource))));
		else
		{
			MXMDExternalResource_V31 *res = static_cast<MXMDExternalResource_V31 *>(externalResource);
			return MXMDTextures::Ptr(new MXMDTextures_V31_Wrap(reinterpret_cast<CASMTHeader_V3 *>(
				data.masterBuffer + data.header->uncachedTexturesOffset),
				res,
				reinterpret_cast<CASMTGroup *>(data.masterBuffer + data.header->cachedTexturesOffset)));
		}
	}

	default:
		return nullptr;
	}
}

MXMDInstances::Ptr MXMD::GetInstances()
{
	switch (data.header->version)
	{
	case MXMDVer1:
	{
		if (!data.header->instancesOffset)
			return nullptr;

		return MXMDInstances::Ptr
		(
			new MXMDInstances_V1_Wrap
			(
				reinterpret_cast<MXMDInstancesHeader_V1 *>(data.header->GetMe() + data.header->instancesOffset)
			)
		);
	}

	default:
		return nullptr;
	}
}

MXMDShaders::Ptr MXMD::GetShaders()
{
	if (!data.header->shadersOffset)
		return nullptr;

	switch (data.header->version)
	{
	case MXMDVer1:
		return MXMDShaders::Ptr
		(
			new MXMDShaders_V1_Wrap
			(
				reinterpret_cast<MXMDShadersHeader_V1 *>(data.header->GetMe() + data.header->shadersOffset)
			)
		);

	default:
		return nullptr;
	}
}

MXMDExternalTextures::Ptr MXMD::GetExternalTextures()
{
	if (!data.header->externalTexturesOffset)
		return nullptr;

	switch (data.header->version)
	{
	case MXMDVer1:
		return MXMDExternalTextures::Ptr
		(
			new MXMDExternalTextures_V1_Wrap
			(
				reinterpret_cast<MXMDExternalTexture_V1 *>(data.header->GetMe() + data.header->externalTexturesOffset), 
				data.header->externalTexturesCount
			)
		);

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