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
#include "MXMD_V1.h"

struct MXMDMeshObject_V3 {
  int ID, skinDescriptor;
  short bufferID, UVFacesID, unk00, materialID;
  int null00[3];
  short unk01, LOD;
  int meshFacesID, null01[3];
};

struct MXMDMeshGroup_V3 {
  int meshesOffset, meshesCount, unk00;
  Vector BBMax, BBMin;
  float boundingRadius;

  MXMDMeshObject_V3 *GetMeshes(char *masterBuffer) {
    return reinterpret_cast<MXMDMeshObject_V3 *>(masterBuffer + meshesOffset);
  }
};

struct MXMDBone_V3 {
  int nameOffset;
  float unk00;
  int unk01, ID, null[2];

  char *GetMe() { return reinterpret_cast<char *>(this); }
  const char *GetBoneName(char *masterBuffer) {
    return masterBuffer + nameOffset;
  }
};

struct MXMDSkinBones_V3 {
  int count1, count2, nodesOffset, nodeTMSOffset, unkOffset00, unkOffset01;

  char *GetMe() { return reinterpret_cast<char *>(this); }
  MXMDBone_V3 *GetBones() {
    return reinterpret_cast<MXMDBone_V3 *>(GetMe() + nodesOffset);
  }
  MXMDTransformMatrix *GetTransforms() {
    return reinterpret_cast<MXMDTransformMatrix *>(GetMe() + nodeTMSOffset);
  }
};

struct MXMDMorphControl_V3 {
  int offset1, offset2, data[5];

  const char *GetName1(char *masterBuffer) { return masterBuffer + offset1; }
  const char *GetName2(char *masterBuffer) { return masterBuffer + offset2; }
};

struct MXMDMorphControls_V3 {
  int offset, count, unk[4];

  char *GetMe() { return reinterpret_cast<char *>(this); }
  MXMDMorphControl_V3 *GetControls() {
    return reinterpret_cast<MXMDMorphControl_V3 *>(GetMe() + offset);
  }
};

struct MXMDModel_V3 {
  int unk00;
  Vector BBMin;
  Vector BBMax;
  int assemblyOffset,
      boneListCount, // assemblyCount?
      null00, nodesOffset, null01[10], unkCount00, null02[10],
      morphControlsOffset, unkOffset00, unkOffset01, unkOffset02, unkOffset03,
      unkCount01;

  char *GetMe() { return reinterpret_cast<char *>(this); }
  MXMDSkinBones_V3 *GetSkinBones() {
    return reinterpret_cast<MXMDSkinBones_V3 *>(GetMe() + nodesOffset);
  }
  MXMDMeshGroup_V3 *GetMeshGroups() {
    return reinterpret_cast<MXMDMeshGroup_V3 *>(GetMe() + assemblyOffset);
  }
  MXMDMorphControls_V3 *GetMorphControls() {
    return !morphControlsOffset ? nullptr
                                : reinterpret_cast<MXMDMorphControls_V3 *>(
                                      GetMe() + morphControlsOffset);
  }
};

struct MXMDVertexBuffer_V3 {
  int offset, count, stride, descriptorsOffset, descriptorsCount, null[3];

  MXMDVertexType *Descriptors(char *masterBuffer) {
    return reinterpret_cast<MXMDVertexType *>(masterBuffer + descriptorsOffset);
  }
  char *Buffer(char *masterBuffer) { return masterBuffer + offset; }
};

struct MXMDFaceBuffer_V3 {
  int offset, count, null[3];

  USVector *Buffer(char *masterBuffer) {
    return reinterpret_cast<USVector *>(masterBuffer + offset);
  }
  ushort *BufferRaw(char *masterBuffer) {
    return reinterpret_cast<ushort *>(masterBuffer + offset);
  }
};

struct MXMDWeightPalette_V3 {
  int baseOffset, offsetID, count, unk00[4];
  char unk01, lod;
  short unk02;
  int unk03[2];
};

struct MXMDMorphBuffer_V3 {
  int offset, count, blocksize;
  short unk, type;
};

struct MXMDMorphDescriptor_V3 {
  int bufferID, targetOffset, targetCount, targetIDOffset, unk01;

  short *GetTargetNameIDs(char *masterBuffer) {
    return reinterpret_cast<short *>(masterBuffer + targetIDOffset);
  }
};

struct MXMDMorphsHeader_V3 {
  int descCount, descOffset, bufferCount, bufferOffset;

  MXMDMorphDescriptor_V3 *GetDescriptors(char *masterBuffer) {
    return reinterpret_cast<MXMDMorphDescriptor_V3 *>(masterBuffer +
                                                      descOffset);
  }
  MXMDMorphBuffer_V3 *GetBufferHeaders(char *masterBuffer) {
    return reinterpret_cast<MXMDMorphBuffer_V3 *>(masterBuffer + bufferOffset);
  }
};

struct MXMDBBufferManager_V3 {
  int numWeightPalettes, weightPaletteOffset;
  short weightBufferID, flags;
  int morphOffset;

  MXMDWeightPalette_V3 *GetWeightPalettes(char *masterBuffer) {
    return reinterpret_cast<MXMDWeightPalette_V3 *>(masterBuffer +
                                                    weightPaletteOffset);
  }
  MXMDMorphsHeader_V3 *GetMorphHeader(char *masterBuffer, char *endBuff) {
    char *mOffset = masterBuffer + morphOffset + (flags == 3 ? 56 : 20);
    return reinterpret_cast<MXMDMorphsHeader_V3 *>(mOffset > endBuff ? nullptr
                                                                     : mOffset);
  }
};

struct MXMDGeometryHeader_V3 {
  int vertexBuffersOffset, vertexBuffersCount, faceBuffersOffset,
      faceBuffersCount;
  short unk00[2];
  int null00[2], unk01, unkOffset00, unkCount00, unk02, bufferSize,
      bufferOffset, voxelizedModelOffset, bufferManagerOffset;

  char *GetMe() { return reinterpret_cast<char *>(this); }
  MXMDVertexBuffer_V3 *GetVertexBuffers() {
    return reinterpret_cast<MXMDVertexBuffer_V3 *>(GetMe() +
                                                   vertexBuffersOffset);
  }
  MXMDFaceBuffer_V3 *GetFaceBuffers() {
    return reinterpret_cast<MXMDFaceBuffer_V3 *>(GetMe() + faceBuffersOffset);
  }
  MXMDBBufferManager_V3 *GetBufferManager() {
    return !bufferManagerOffset ? nullptr
                                : reinterpret_cast<MXMDBBufferManager_V3 *>(
                                      GetMe() + bufferManagerOffset);
  }
};

struct CASMTHeader_V3 : CASMTHeader {
  int *GetGroupDataOffsets() { return GetGroupIDsOffsets() + numGroups; }
  int *GetGroupDataUncompSizes() { return GetGroupDataOffsets() + numGroups; }
  int *GetGroupDataSizes() { return GetGroupDataUncompSizes() + numGroups; }
};

struct MXMDTextureLink_V3 : MXMDTextureLink_V1 {
  int unk01;
};

struct MXMDMaterial_V3 {
  int nameOffset, unk00;
  float unk01[6];
  int texturesOffset, texturesCount, unk02[9], linksOffset, linksCount,
      unk03[8];

  const char *GetName(char *masterBuffer) { return masterBuffer + nameOffset; }
  MXMDTextureLink_V3 *GetTextureLinks(char *masterBuffer) {
    return reinterpret_cast<MXMDTextureLink_V3 *>(masterBuffer +
                                                  texturesOffset);
  }
};

struct MXMDMaterialsHeader_V3 : MXMDMaterialsHeader_V1 {
  MXMDMaterial_V3 *GetMaterials() {
    return reinterpret_cast<MXMDMaterial_V3 *>(GetMe() + materialsOffset);
  }
};