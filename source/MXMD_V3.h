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

struct MXMDMeshObject_V3
{
	int ID,
		skinDescriptor;
	short bufferID,
		UVFacesID,
		unk00,
		materialID;
	int null00[3];
	short unk01,
		LOD;
	int meshFacesID,
		null01[3];
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

struct MXMDMeshGroup_V3
{
	int meshesOffset,
		meshesCount,
		unk00;
	Vector BBMax,
		BBMin;
	float boundingRadius;

	ES_FORCEINLINE MXMDMeshObject_V3 *GetMeshes(char *masterBuffer) { return reinterpret_cast<MXMDMeshObject_V3 *>(masterBuffer + meshesOffset); }
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

struct MXMDBone_V3
{
	int nameOffset;
	float unk00;
	int unk01,
		ID,
		null[2];

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE const char *GetBoneName(char *masterBuffer) { return masterBuffer + nameOffset; }
};

struct MXMDSkinBones_V3
{
	int count1,
		count2,
		nodesOffset,
		nodeTMSOffset,
		unkOffset00,
		unkOffset01;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE MXMDBone_V3 *GetBones() { return reinterpret_cast<MXMDBone_V3 *>(GetMe() + nodesOffset); }
	ES_FORCEINLINE MXMDTransformMatrix *GetTransforms() { return reinterpret_cast<MXMDTransformMatrix *>(GetMe() + nodeTMSOffset); }
};

class MXMDBone_V3_Wrap : public MXMDBone
{
	MXMDBone_V3 *data;
	MXMDTransformMatrix *transform;
	char *masterBuffer;
public:
	MXMDBone_V3_Wrap(MXMDBone_V3 *input, char *inputBuff, MXMDTransformMatrix *inTransforms) : data(input), masterBuffer(inputBuff), transform(inTransforms + data->ID){}

	const char *GetName() const { return data->GetBoneName(masterBuffer); }
	int GetParentID() const { return -1; }
	const MXMDTransformMatrix *GetAbsTransform() const { return transform; }
	const MXMDTransformMatrix *GetTransform() const { return nullptr; }
};

struct MXMDMorphControl_V3
{
	int offset1,
		offset2,
		data[5];

	ES_FORCEINLINE const char *GetName1(char *masterBuffer) { return masterBuffer + offset1; }
	ES_FORCEINLINE const char *GetName2(char *masterBuffer) { return masterBuffer + offset2; }
};

struct MXMDMorphControls_V3
{
	int offset,
		count,
		unk[4];

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE MXMDMorphControl_V3 *GetControls() { return reinterpret_cast<MXMDMorphControl_V3 *>(GetMe() + offset); }
};

struct MXMDModel_V3
{
	int unk00;
	Vector BBMin;
	Vector BBMax;
	int assemblyOffset,
		boneListCount, //assemblyCount?
		null00,
		nodesOffset,
		null01[10],
		unkCount00,
		null02[10],
		morphControlsOffset,
		unkOffset00,
		unkOffset01,
		unkOffset02,
		unkOffset03,
		unkCount01;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE MXMDSkinBones_V3 *GetSkinBones() { return reinterpret_cast<MXMDSkinBones_V3 *>(GetMe() + nodesOffset); }
	ES_FORCEINLINE MXMDMeshGroup_V3 *GetMeshGroups() { return reinterpret_cast<MXMDMeshGroup_V3 *>(GetMe() + assemblyOffset); }
	ES_FORCEINLINE MXMDMorphControls_V3 *GetMorphControls() { return !morphControlsOffset ? nullptr : reinterpret_cast<MXMDMorphControls_V3 *>(GetMe() + morphControlsOffset); }
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

struct MXMDVertexBuffer_V3
{
	int offset,
		count,
		stride,
		descriptorsOffset,
		descriptorsCount,
		null[3];

	ES_FORCEINLINE MXMDVertexType *Descriptors(char *masterBuffer) { return reinterpret_cast<MXMDVertexType *>(masterBuffer + descriptorsOffset); }
	ES_FORCEINLINE char *Buffer(char *masterBuffer) { return masterBuffer + offset; }
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

struct MXMDFaceBuffer_V3
{
	int offset,
		count,
		null[3];

	ES_FORCEINLINE USVector *Buffer(char *masterBuffer) { return reinterpret_cast<USVector *>(masterBuffer + offset); }
	ES_FORCEINLINE ushort *BufferRaw(char *masterBuffer) { return reinterpret_cast<ushort *>(masterBuffer + offset); }
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

struct MXMDWeightPalette_V3
{
	int baseOffset,
		offsetID,
		count,
		unk00[4];
	char unk01,
		lod;
	short unk02;
	int unk03[2];
};

struct MXMDMorphBuffer_V3
{
	int offset,
		count,
		blocksize;
	short unk,
		type;
};

struct MXMDMorphDescriptor_V3
{
	int bufferID,
		targetOffset,
		targetCount,
		targetIDOffset,
		unk01;

	ES_FORCEINLINE short *GetTargetNameIDs(char *masterBuffer) { return reinterpret_cast<short *>(masterBuffer + targetIDOffset); }
};

struct MXMDMorphsHeader_V3
{
	int descCount,
		descOffset,
		bufferCount,
		bufferOffset;

	ES_FORCEINLINE MXMDMorphDescriptor_V3 *GetDescriptors(char *masterBuffer) { return reinterpret_cast<MXMDMorphDescriptor_V3 *>(masterBuffer + descOffset); }
	ES_FORCEINLINE MXMDMorphBuffer_V3 *GetBufferHeaders(char *masterBuffer) { return reinterpret_cast<MXMDMorphBuffer_V3 *>(masterBuffer + bufferOffset); }
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

struct MXMDBBufferManager_V3
{
	int numWeightPalettes,
		weightPaletteOffset;
	short weightBufferID,
		flags;
	int morphOffset;

	ES_FORCEINLINE MXMDWeightPalette_V3 *GetWeightPalettes(char *masterBuffer) { return reinterpret_cast<MXMDWeightPalette_V3 *>(masterBuffer + weightPaletteOffset); }
	ES_FORCEINLINE MXMDMorphsHeader_V3 *GetMorphHeader(char *masterBuffer, char *endBuff) 
	{ 
		char *mOffset = masterBuffer + morphOffset + (flags == 3 ? 56 : 20);
		return reinterpret_cast<MXMDMorphsHeader_V3 *>(mOffset > endBuff ? nullptr : mOffset);
	}
};

struct MXMDGeometryHeader_V3
{
	int vertexBuffersOffset,
		vertexBuffersCount,
		faceBuffersOffset,
		faceBuffersCount;
	short unk00[2];
	int null00[2],
		unk01,
		unkOffset00,
		unkCount00,
		unk02,
		bufferSize,
		bufferOffset,
		voxelizedModelOffset,
		bufferManagerOffset;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
	ES_FORCEINLINE MXMDVertexBuffer_V3 *GetVertexBuffers() { return reinterpret_cast<MXMDVertexBuffer_V3 *>(GetMe() + vertexBuffersOffset); }
	ES_FORCEINLINE MXMDFaceBuffer_V3 *GetFaceBuffers() { return reinterpret_cast<MXMDFaceBuffer_V3 *>(GetMe() + faceBuffersOffset); }
	ES_FORCEINLINE MXMDBBufferManager_V3 *GetBufferManager() { return !bufferManagerOffset ? nullptr : reinterpret_cast<MXMDBBufferManager_V3 *>(GetMe() + bufferManagerOffset); }
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

struct CASMTHeader_V3 : CASMTHeader
{
	ES_FORCEINLINE int *GetGroupDataOffsets() { return GetGroupIDsOffsets() + numGroups; }
	ES_FORCEINLINE int *GetGroupDataUncompSizes() { return GetGroupDataOffsets() + numGroups; }
	ES_FORCEINLINE int *GetGroupDataSizes() { return GetGroupDataUncompSizes() + numGroups; }
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

struct MXMDTextureLink_V3 : MXMDTextureLink_V1
{
	int unk01;
};

struct MXMDMaterial_V3
{
	int nameOffset,
		unk00;
	float unk01[6];
	int texturesOffset,
		texturesCount,
		unk02[9],
		linksOffset,
		linksCount,
		unk03[8];
	
	ES_FORCEINLINE const char *GetName(char *masterBuffer) { return masterBuffer + nameOffset; }
	ES_FORCEINLINE MXMDTextureLink_V3 *GetTextureLinks(char *masterBuffer) { return reinterpret_cast<MXMDTextureLink_V3 *>(masterBuffer + texturesOffset); }
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

struct MXMDMaterialsHeader_V3 : MXMDMaterialsHeader_V1
{
	ES_FORCEINLINE MXMDMaterial_V3 *GetMaterials() { return reinterpret_cast<MXMDMaterial_V3 *>(GetMe() + materialsOffset); }
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