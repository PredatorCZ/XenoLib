/*      Xenoblade Engine Format Library
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
#include "MXMD_Internal.h"

struct MXMDMeshObject_V1 {
  int flags, skinDescriptor, bufferID, UVFacesID, unk00, materialID, null00[2],
      gibID, null01, meshFacesID, null02[5];

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct MXMDMeshGroup_V1 {
  int meshesOffset, meshesCount,
      bufferID; // originally null, now custom value
  Vector BBMax, BBMin;
  float boundingRadius;
  int null[7];

  ES_FORCEINLINE void SwapEndian(char *masterBuffer) {
    _ArraySwap<int>(*this);

    MXMDMeshObject_V1 *meshes = GetMeshes(masterBuffer);

    for (int t = 0; t < meshesCount; t++)
      meshes[t].SwapEndian();
  }
  ES_FORCEINLINE MXMDMeshObject_V1 *GetMeshes(char *masterBuffer) {
    return reinterpret_cast<MXMDMeshObject_V1 *>(masterBuffer + meshesOffset);
  }
};

struct MXMDSkinBones_V1 {
  int offset, count;

  ES_FORCEINLINE void SwapEndian(char *masterBuffer) {
    _ArraySwap<int>(*this);

    short *ids = GetIDs(masterBuffer);
    float *floats = GetFloats(masterBuffer);

    for (int r = 0; r < count; r++) {
      FByteswapper(ids[r]);
      FByteswapper(floats[r]);
    }
  }

  ES_FORCEINLINE short *GetIDs(char *masterBuffer) {
    return reinterpret_cast<short *>(masterBuffer + offset);
  }
  ES_FORCEINLINE float *GetFloats(char *masterBuffer) {
    return reinterpret_cast<float *>(GetIDs(masterBuffer) + count +
                                     (count & 1));
  }
};

struct MXMDBone_V1 {
  int nameOffset, parentID, firstChild, lastChild, unk00;
  Vector position;
  float unk01[6];
  MXMDTransformMatrix absTransform;
  MXMDTransformMatrix transform;

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
  ES_FORCEINLINE const char *GetBoneName(char *masterBuffer) {
    return masterBuffer + nameOffset;
  }
};

struct MXMDModel_V1 {
  Vector BBMin;
  Vector BBMax;
  int assemblyOffset, assemblyCount, boneListOffset, boneListCount, unk00[3],
      attachmentsOffset2, unk01[6], nodesOffset, nodesCount, floatArrayOffset,
      floatArrayCount, unk02, boneNamesOffset, boneNamesCount, unk03,
      attachmentsOffset, attachmentsCount, unkOffset00, unk04[4], unkOffset01,
      unk05;

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE void SwapEndian() {
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

    int *_boneNames = GetBoneNamesOffsets();

    if (_boneNames)
      for (int t = 0; t < boneNamesCount; t++)
        FByteswapper(_boneNames[t]);

  }
  ES_FORCEINLINE MXMDMeshGroup_V1 *GetMeshGroups() {
    return reinterpret_cast<MXMDMeshGroup_V1 *>(GetMe() + assemblyOffset);
  }
  ES_FORCEINLINE MXMDBone_V1 *GetBones() {
    return reinterpret_cast<MXMDBone_V1 *>(GetMe() + nodesOffset);
  }
  ES_FORCEINLINE MXMDSkinBones_V1 *GetSkinBones() {
    return !boneListOffset
               ? nullptr
               : reinterpret_cast<MXMDSkinBones_V1 *>(GetMe() + boneListOffset);
  }
  ES_FORCEINLINE int *GetBoneNamesOffsets() {
    return !boneNamesOffset ? nullptr
                           : reinterpret_cast<int *>(GetMe() + boneNamesOffset);
  }
};

struct MXMDVertexBuffer_V1 {
  int offset, count, stride, descriptorsOffset, descriptorsCount, null;

  ES_FORCEINLINE void SwapEndian(char *masterBuffer) {
    _ArraySwap<int>(*this);

    MXMDVertexType *desc = Descriptors(masterBuffer);

    for (int d = 0; d < descriptorsCount; d++)
      desc[d].SwapEndian();
  }
  ES_FORCEINLINE MXMDVertexType *Descriptors(char *masterBuffer) {
    return reinterpret_cast<MXMDVertexType *>(masterBuffer + descriptorsOffset);
  }
  ES_FORCEINLINE char *Buffer(char *masterBuffer) {
    return masterBuffer + offset;
  }
};

struct MXMDFaceBuffer_V1 {
  int offset, count, null;

  ES_FORCEINLINE void SwapEndian(char *masterBuffer) {
    _ArraySwap<int>(*this);

    ushort *buff = BufferRaw(masterBuffer);

    for (int d = 0; d < count; d++)
      FByteswapper(buff[d]);
  }
  ES_FORCEINLINE USVector *Buffer(char *masterBuffer) {
    return reinterpret_cast<USVector *>(masterBuffer + offset);
  }
  ES_FORCEINLINE ushort *BufferRaw(char *masterBuffer) {
    return reinterpret_cast<ushort *>(masterBuffer + offset);
  }
};

struct MXMDGeometryHeader_V1 {
  int vertexBuffersOffset, vertexBuffersCount, faceBuffersOffset,
      faceBuffersCount;
  short mergeData[16];

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE void SwapEndian() {
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

  ES_FORCEINLINE MXMDVertexBuffer_V1 *GetVertexBuffers() {
    return reinterpret_cast<MXMDVertexBuffer_V1 *>(GetMe() +
                                                   vertexBuffersOffset);
  }
  ES_FORCEINLINE MXMDFaceBuffer_V1 *GetFaceBuffers() {
    return reinterpret_cast<MXMDFaceBuffer_V1 *>(GetMe() + faceBuffersOffset);
  }
};

struct CASMTTexture {
  int unk, size, offset, nameOffset;

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct CASMTGroup {
  int count, blen, unk, size;

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE void SwapEndian() {
    _ArraySwap<int>(*this);

    CASMTTexture *texts = GetTextures();

    for (int t = 0; t < count; t++)
      texts[t].SwapEndian();
  }
  ES_FORCEINLINE CASMTTexture *GetTextures() {
    return reinterpret_cast<CASMTTexture *>(this + 1);
  }
};

struct CASMTHeader {
  int dataOffset, numGroups;

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE void SwapEndian() {
    _ArraySwap<int>(*this);

    int *gOffsets = GetGroupOffsets();
    int *gidOffsets = GetGroupIDsOffsets();
    int *gDOffsets = GetGroupDataOffsets();
    int *gDSizes = GetGroupDataSizes();

    for (int v = 0; v < numGroups; v++) {
      FByteswapper(gOffsets[v]);
      FByteswapper(gidOffsets[v]);
      FByteswapper(gDOffsets[v]);
      FByteswapper(gDSizes[v]);
      GetGroup(v)->SwapEndian();

      for (int g = 0; g < GetGroup(v)->count; g++)
        FByteswapper(GetGroupTextureIDs(v)[g]);
    }
  }

  ES_FORCEINLINE int *GetGroupOffsets() {
    return reinterpret_cast<int *>(this + 1);
  }
  ES_FORCEINLINE int *GetGroupIDsOffsets() { return GetGroupOffsets() + 2; }
  ES_FORCEINLINE int *GetGroupDataOffsets() { return GetGroupIDsOffsets() + 2; }
  ES_FORCEINLINE int *GetGroupDataSizes() { return GetGroupDataOffsets() + 2; }

  ES_FORCEINLINE CASMTGroup *GetGroup(int id) {
    return reinterpret_cast<CASMTGroup *>(GetMe() + GetGroupOffsets()[id]);
  }
  ES_FORCEINLINE short *GetGroupTextureIDs(int id) {
    return reinterpret_cast<short *>(GetMe() + GetGroupIDsOffsets()[id]);
  }
};

struct MXMDTextureLink_V1 {
  short textureID, unk;

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<short>(*this); }
};

struct MXMDMaterial_V1 {
  int nameOffset, unk00;
  float unk01[13];
  int texturesOffset, texturesCount, unk02[9], linksOffset, linksCount, null[6];

  ES_FORCEINLINE void SwapEndian(char *masterBuffer) {
    _ArraySwap<int>(*this);

    MXMDTextureLink_V1 *links = GetTextureLinks(masterBuffer);

    for (int t = 0; t < texturesCount; t++)
      links[t].SwapEndian();
  }
  ES_FORCEINLINE const char *GetName(char *masterBuffer) {
    return masterBuffer + nameOffset;
  }
  ES_FORCEINLINE MXMDTextureLink_V1 *GetTextureLinks(char *masterBuffer) {
    return reinterpret_cast<MXMDTextureLink_V1 *>(masterBuffer +
                                                  texturesOffset);
  }
};

struct MXMDMaterialsHeader_V1 {
  int materialsOffset, materialsCount, unk00[2], floatArrayOffset,
      floatArrayCount, uintArrayOffset, uintArrayCount, unk01, unkOffset00,
      unkOffset01, unkCount01, unk02[3], unkOffset02, unk03[3], unkOffset03,
      unk04[2], unkOffset04, unk05[3];

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE MXMDMaterial_V1 *GetMaterials() {
    return reinterpret_cast<MXMDMaterial_V1 *>(GetMe() + materialsOffset);
  }
  ES_FORCEINLINE void SwapEndian() {
    _ArraySwap<int>(*this);

    MXMDMaterial_V1 *mats = GetMaterials();

    for (int m = 0; m < materialsCount; m++)
      mats[m].SwapEndian(GetMe());
  }
};

struct MXMDInstanceItem_V1 {
  float unk[8];
  int meshGroup;

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct MXMDInstanceLookup_V1 {
  int groupIDStart, numGroups;

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct MXMDInstanceMatrix_V1 {
  MXMDTransformMatrix mtx;
  Vector pos00;
  Vector4 pos01;
  int lookupIndex, null00, unk, null01;

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct MXMDInstancesHeader_V1 {
  int null00, lookupsCount, lookupsOffset, groupsCount, groupsOffset,
      matricesCount, matricesOffset, unk01[16], null01[6];

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE MXMDInstanceMatrix_V1 *GetMatrices() {
    return reinterpret_cast<MXMDInstanceMatrix_V1 *>(GetMe() + matricesOffset);
  }
  ES_FORCEINLINE MXMDInstanceLookup_V1 *GetLookups() {
    return reinterpret_cast<MXMDInstanceLookup_V1 *>(GetMe() + lookupsOffset);
  }
  ES_FORCEINLINE MXMDInstanceItem_V1 *GetInstanceItems() {
    return reinterpret_cast<MXMDInstanceItem_V1 *>(GetMe() + groupsOffset);
  }
  ES_FORCEINLINE void SwapEndian() {
    _ArraySwap<int>(*this);

    MXMDInstanceMatrix_V1 *mats = GetMatrices();

    for (int m = 0; m < matricesCount; m++)
      mats[m].SwapEndian();

    MXMDInstanceLookup_V1 *lookups = GetLookups();

    for (int m = 0; m < lookupsCount; m++)
      lookups[m].SwapEndian();

    MXMDInstanceItem_V1 *items = GetInstanceItems();

    for (int m = 0; m < groupsCount; m++)
      items[m].SwapEndian();
  }
};

struct MXMDTerrainBufferLookup_V1 {
  float unk00[6];
  int bufferIndex[2];
  int null;

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct MXMDTerrainBufferLookupHeader_V1 {
  int bufferLookupsOffset, bufferLookupCount, groupIndicesOffset,
      groupIndicesCount, unk00, null00[6];

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE ushort *GetGroupIndices() {
    return reinterpret_cast<ushort *>(GetMe() + groupIndicesOffset);
  }
  ES_FORCEINLINE MXMDTerrainBufferLookup_V1 *GetBufferLookups() {
    return reinterpret_cast<MXMDTerrainBufferLookup_V1 *>(GetMe() +
                                                          bufferLookupsOffset);
  }

  ES_FORCEINLINE void SwapEndian() {
    _ArraySwap<int>(*this);

    ushort *ids = GetGroupIndices();

    for (int m = 0; m < groupIndicesCount; m++)
      FByteswapper(ids[m]);

    MXMDTerrainBufferLookup_V1 *lookups = GetBufferLookups();

    for (int m = 0; m < bufferLookupCount; m++)
      lookups[m].SwapEndian();
  }

  ES_FORCEINLINE void RSwapEndian() {
    ushort *ids = GetGroupIndices();

    for (int m = 0; m < groupIndicesCount; m++)
      FByteswapper(ids[m]);

    MXMDTerrainBufferLookup_V1 *lookups = GetBufferLookups();

    for (int m = 0; m < bufferLookupCount; m++)
      lookups[m].SwapEndian();

    _ArraySwap<int>(*this);
  }
};

struct MXMDShaderItem_V1 {
  int offset, size, null[2];

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<int>(*this); }
};

struct MXMDShadersHeader_V1 {
  int arrayOffset, numShaders, unk, null[5];

  ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
  ES_FORCEINLINE MXMDShaderItem_V1 *GetShaderItems() {
    return reinterpret_cast<MXMDShaderItem_V1 *>(GetMe() + arrayOffset);
  }

  ES_FORCEINLINE void SwapEndian() {
    _ArraySwap<int>(*this);

    MXMDShaderItem_V1 *items = GetShaderItems();

    for (int m = 0; m < numShaders; m++)
      items[m].SwapEndian();
  }
};

struct MXMDExternalTexture_V1 {
  short textureID, containerID, externalTextureID, unk;

  ES_FORCEINLINE void SwapEndian() { _ArraySwap<short>(*this); }
};