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

struct MXMDMeshObject_V1
{
	int flags,
		skinDescriptor,
		bufferID,
		UVFacesID,
		unk00,
		materialID,
		null00[2],
		gibID,
		null01,
		meshFacesID,
		null02[5];

	ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

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

struct MXMDMeshGroup_V1
{
	int meshesOffset,
		meshesCount,
		unk00;
	Vector BBMax, 
		BBMin;
	float boundingRadius;

	ES_FORCEINLINE void SwapEndian(char *masterBuffer)
	{ 
		_ArraySwap<int>(*this); 

		MXMDMeshObject_V1 *meshes = GetMeshes(masterBuffer);

		for (int t = 0; t < meshesCount; t++)
			meshes[t].SwapEndian();
	}
	ES_FORCEINLINE MXMDMeshObject_V1 *GetMeshes(char *masterBuffer) { return reinterpret_cast<MXMDMeshObject_V1 *>(masterBuffer + meshesOffset); }
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

struct MXMDSkinBones_V1
{
	int offset,
		count;

	ES_FORCEINLINE void SwapEndian(char *masterBuffer)
	{ 
		_ArraySwap<int>(*this); 

		short *ids = GetIDs(masterBuffer);
		float *floats = GetFloats(masterBuffer);
		int *nameOffsets = GetBoneNameOffsets(masterBuffer);

		for (int r = 0; r < count; r++)
		{
			FByteswapper(ids[r]);
			FByteswapper(floats[r]);
			FByteswapper(nameOffsets[r]);
		}
	}

	ES_FORCEINLINE short *GetIDs(char *masterBuffer) { return reinterpret_cast<short *>(masterBuffer + offset); }
	ES_FORCEINLINE float *GetFloats(char *masterBuffer) { return reinterpret_cast<float *>(GetIDs(masterBuffer) + count + (count & 1)); }
	ES_FORCEINLINE int *GetBoneNameOffsets(char *masterBuffer) { return reinterpret_cast<int *>(GetFloats(masterBuffer) + count); }
};

struct MXMDBone_V1
{
	int nameOffset,
		parentID,
		firstChild,
		lastChild,
		unk00;
	Vector position;
	float unk01[6];
	MXMDTransformMatrix absTransform;
	MXMDTransformMatrix transform;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
	ES_FORCEINLINE const char *GetBoneName(char *masterBuffer) { return masterBuffer + nameOffset; }
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

struct MXMDModel_V1
{
	Vector BBMin;
	Vector BBMax;
	int assemblyOffset,
		assemblyCount,
		boneListOffset,
		boneListCount,
		unk00[3],
		attachmentsOffset2,
		unk01[6],
		nodesOffset,
		nodesCount,
		floatArrayOffset,
		floatArrayCount,
		unk02,
		boneNamesOffset,
		boneNamesCount,
		unk03,
		attachmentsOffset,
		attachmentsCount,
		unkOffset00,
		unk04[4],
		unkOffset01,
		unk05[3];

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE void SwapEndian() 
	{
		_ArraySwap<int>(*this); 

		MXMDMeshGroup_V1 *mGroups = GetMeshGroups();

		for (int t = 0; t < assemblyCount; t++)
			mGroups[t].SwapEndian(GetMe());

		MXMDBone_V1 *bones = GetBones();

		for (int t = 0; t < nodesCount; t++)
			bones[t].SwapEndian();

		MXMDSkinBones_V1 *skinBones = GetSkinBones();

		if (skinBones)
			skinBones->SwapEndian(GetMe());
	}
	ES_FORCEINLINE MXMDMeshGroup_V1 *GetMeshGroups() { return reinterpret_cast<MXMDMeshGroup_V1 *>(GetMe() + assemblyOffset); }
	ES_FORCEINLINE MXMDBone_V1 *GetBones() { return reinterpret_cast<MXMDBone_V1 *>(GetMe() + nodesOffset); }
	ES_FORCEINLINE MXMDSkinBones_V1 *GetSkinBones() { return !boneListOffset ? nullptr : reinterpret_cast<MXMDSkinBones_V1 *>(GetMe() + boneListOffset); }
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

		MXMDSkinBones_V1 *sBones = data->GetSkinBones();

		if (!sBones)
			return;

		short *ids = sBones->GetIDs(data->GetMe());
		int *nameOffsets = sBones->GetBoneNameOffsets(data->GetMe());

		remapBones.resize(sBones->count);

		for (int r = 0; r < sBones->count; r++)
		{
			short cid = ids[r];
			const char *bneName = reinterpret_cast<const char *>(nameOffsets) + nameOffsets[cid];

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

struct MXMDVertexBuffer_V1
{
	int offset,
		count,
		stride,
		descriptorsOffset,
		descriptorsCount,
		null;

	ES_FORCEINLINE void SwapEndian(char *masterBuffer)
	{ 
		_ArraySwap<int>(*this); 

		MXMDVertexType *desc = Descriptors(masterBuffer);

		for (int d = 0; d < descriptorsCount; d++)
			desc[d].SwapEndian();
	}
	ES_FORCEINLINE MXMDVertexType *Descriptors(char *masterBuffer) { return reinterpret_cast<MXMDVertexType *>(masterBuffer + descriptorsOffset); }
	ES_FORCEINLINE char *Buffer(char *masterBuffer) { return masterBuffer + offset; }
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

struct MXMDFaceBuffer_V1
{
	int offset,
		count,
		null;

	ES_FORCEINLINE void SwapEndian(char *masterBuffer)
	{
		_ArraySwap<int>(*this);

		ushort *buff = BufferRaw(masterBuffer);

		for (int d = 0; d < count; d++)
			FByteswapper(buff[d]);

	}
	ES_FORCEINLINE USVector *Buffer(char *masterBuffer) { return reinterpret_cast<USVector *>(masterBuffer + offset); }
	ES_FORCEINLINE ushort *BufferRaw(char *masterBuffer) { return reinterpret_cast<ushort *>(masterBuffer + offset); }
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

struct MXMDGeometryHeader_V1
{
	int vertexBuffersOffset,
		vertexBuffersCount,
		faceBuffersOffset,
		faceBuffersCount;
	short mergeData[16];

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE void SwapEndian() 
	{
		FByteswapper(vertexBuffersOffset);
		FByteswapper(vertexBuffersCount);
		FByteswapper(faceBuffersOffset);
		FByteswapper(faceBuffersCount);

		for (short &s : mergeData)
			FByteswapper(s);

		MXMDVertexBuffer_V1 *vBuffers = GetVertexBuffers();

		for (int v = 0; v < vertexBuffersCount; v++)
			vBuffers[v].SwapEndian(GetMe());

		MXMDFaceBuffer_V1 *fBuffers = GetFaceBuffers();

		for (int v = 0; v < faceBuffersCount; v++)
			fBuffers[v].SwapEndian(GetMe());
	}

	ES_FORCEINLINE MXMDVertexBuffer_V1 *GetVertexBuffers() { return reinterpret_cast<MXMDVertexBuffer_V1 *>(GetMe() + vertexBuffersOffset); }
	ES_FORCEINLINE MXMDFaceBuffer_V1 *GetFaceBuffers() { return reinterpret_cast<MXMDFaceBuffer_V1 *>(GetMe() + faceBuffersOffset); }
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

struct CASMTTexture
{
	int unk,
		size,
		offset,
		nameOffset;

	ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct CASMTGroup
{
	int count,
		blen,
		unk,
		size;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE void SwapEndian() 
	{ 
		_ArraySwap<int>(*this); 

		CASMTTexture *texts = GetTextures();

		for (int t = 0; t < count; t++)
			texts[t].SwapEndian();
			
	}
	ES_FORCEINLINE CASMTTexture *GetTextures() { return reinterpret_cast<CASMTTexture *>(this + 1); }
};

struct CASMTHeader
{
	int dataOffset,
		numGroups;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE void SwapEndian()
	{
		_ArraySwap<int>(*this);

		int *gOffsets = GetGroupOffsets();
		int *gidOffsets = GetGroupIDsOffsets();
		int *gDOffsets = GetGroupDataOffsets();
		int *gDSizes = GetGroupDataSizes();

		for (int v = 0; v < numGroups; v++)
		{
			FByteswapper(gOffsets[v]);
			FByteswapper(gidOffsets[v]);
			FByteswapper(gDOffsets[v]);
			FByteswapper(gDSizes[v]);
			GetGroup(v)->SwapEndian();

			for (int g = 0; g < GetGroup(v)->count; g++)
				FByteswapper(GetGroupTextureIDs(v)[g]);
		}

	}

	ES_FORCEINLINE int *GetGroupOffsets() { return reinterpret_cast<int *>(this + 1); }
	ES_FORCEINLINE int *GetGroupIDsOffsets() { return GetGroupOffsets() + 2; }
	ES_FORCEINLINE int *GetGroupDataOffsets() { return GetGroupIDsOffsets() + 2; }
	ES_FORCEINLINE int *GetGroupDataSizes() { return GetGroupDataOffsets() + 2; }

	ES_FORCEINLINE CASMTGroup *GetGroup(int id) { return reinterpret_cast<CASMTGroup *>(GetMe() + GetGroupOffsets()[id]); }
	ES_FORCEINLINE short *GetGroupTextureIDs(int id) { return reinterpret_cast<short *>(GetMe() + GetGroupIDsOffsets()[id]); }
};

struct MXMDTextureLink_V1
{
	short textureID,
		unk;

	ES_FORCEINLINE void SwapEndian() { _ArraySwap<short>(*this); }
};

struct MXMDMaterial_V1
{
	int nameOffset,
		unk00;
	float unk01[13];
	int texturesOffset,
		texturesCount,
		unk02[9],
		linksOffset,
		linksCount,
		null[6];

	ES_FORCEINLINE void SwapEndian(char *masterBuffer) 
	{
		_ArraySwap<int>(*this);

		MXMDTextureLink_V1 *links = GetTextureLinks(masterBuffer);

		for (int t = 0; t < texturesCount; t++)
			links[t].SwapEndian();

	}
	ES_FORCEINLINE const char *GetName(char *masterBuffer) { return masterBuffer + nameOffset; }
	ES_FORCEINLINE MXMDTextureLink_V1 *GetTextureLinks(char *masterBuffer) { return reinterpret_cast<MXMDTextureLink_V1 *>(masterBuffer + texturesOffset); }
};

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

struct MXMDMaterialsHeader_V1
{
	int materialsOffset,
		materialsCount,
		unk00[2],
		floatArrayOffset,
		floatArrayCount,
		uintArrayOffset,
		uintArrayCount,
		unk01,
		unkOffset00,
		unkOffset01,
		unkCount01,
		unk02[3],
		unkOffset02,
		unk03[3],
		unkOffset03,
		unk04[2],
		unkOffset04,
		unk05[3];

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE MXMDMaterial_V1 *GetMaterials() { return reinterpret_cast<MXMDMaterial_V1 *>(GetMe() + materialsOffset); }
	ES_FORCEINLINE void SwapEndian() 
	{ 
		_ArraySwap<int>(*this); 

		MXMDMaterial_V1 *mats = GetMaterials();

		for (int m = 0; m < materialsCount; m++)
			mats[m].SwapEndian(GetMe());
	}
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