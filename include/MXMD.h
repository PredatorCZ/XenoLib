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
#include "XenoLibAPI.h"
#include "datas/vectors.hpp"
#include <memory>
#include <vector>

struct MXMDHeader;

class MXMDMeshObject {
public:
  typedef std::unique_ptr<MXMDMeshObject> Ptr;

  virtual int GetMeshFacesID() const = 0;
  virtual int GetUVFacesID() const = 0;
  virtual int GetBufferID() const = 0;
  virtual int GetMaterialID() const = 0;
  virtual int GetSkinDesc() const = 0;
  virtual int GetGibID() const = 0;
  virtual int GetLODID() const = 0;
  virtual ~MXMDMeshObject() {}
};

class MXMDMeshGroup {
public:
  typedef std::unique_ptr<MXMDMeshGroup> Ptr;

  virtual MXMDMeshObject::Ptr GetMeshObject(int id) const = 0;
  virtual int GetNumMeshObjects() const = 0;
  virtual ~MXMDMeshGroup() {}
};

struct MXMDTransformMatrix {
  Vector4 m[4];
};

class MXMDBone {
public:
  typedef std::unique_ptr<MXMDBone> Ptr;

  virtual const char *GetName() const = 0;
  virtual int GetParentID() const = 0;
  virtual const MXMDTransformMatrix *GetAbsTransform() const = 0;
  virtual const MXMDTransformMatrix *GetTransform() const = 0;
  virtual ~MXMDBone() {}
};

class MXMDModel {
public:
  typedef std::unique_ptr<MXMDModel> Ptr;

  virtual MXMDMeshGroup::Ptr GetMeshGroup(int id) const = 0;
  virtual int GetNumMeshGroups() const = 0;

  virtual MXMDBone::Ptr GetBone(int id) const = 0;
  virtual int GetNumBones() const = 0;

  virtual MXMDBone::Ptr GetSkinBone(int id) const = 0;
  virtual int GetNumSkinBones() const = 0;

  virtual const char *GetMorphName(int id) const = 0;

  virtual void SwapEndian() {}
  virtual ~MXMDModel() {}
};

class MXMDMaterial {
public:
  typedef std::unique_ptr<MXMDMaterial> Ptr;

  virtual int GetNumTextures() const = 0;
  virtual int GetTextureIndex(int id) const = 0;
  virtual const char *GetName() const = 0;
  virtual ~MXMDMaterial() {}
};

class MXMDMaterials {
public:
  typedef std::unique_ptr<MXMDMaterials> Ptr;

  virtual MXMDMaterial::Ptr GetMaterial(int id) const = 0;
  virtual int GetNumMaterials() const = 0;

  virtual void SwapEndian() {}
  virtual ~MXMDMaterials() {}
};

enum MXMDVertexDescriptorType {
  MXMD_POSITION,
  MXMD_WEIGHT32,
  MXMD_BONEID,
  MXMD_WEIGHTID,
  MXMD_UV1 = 5,
  MXMD_UV2,
  MXMD_UV3,
  MXMD_NORMAL32 = 15,
  MXMD_TANGENT16, // NoFunc
  MXMD_VERTEXCOLOR,
  MXMD_NORMAL = 28,
  MXMD_TANGENT, // NoFunc
  MXMD_NORMAL2 = 32,
  MXMD_REFLECTION, // NoFunc
  MXMD_WEIGHT16 = 41,
  MXMD_BONEID2,
  MXMD_NORMALMORPH,
  MXMD_MORPHVERTEXID
};

class MXMDVertexDescriptor {
public:
  typedef std::unique_ptr<MXMDVertexDescriptor> Ptr;

  virtual void Evaluate(int at, void *data) {}
  virtual MXMDVertexDescriptorType Type() const = 0;
  virtual int Size() const = 0;
  virtual ~MXMDVertexDescriptor() {}
};

class MXMDVertexBuffer {
public:
  typedef std::unique_ptr<MXMDVertexBuffer> Ptr;
  typedef std::vector<MXMDVertexDescriptor::Ptr> DescriptorCollection;

  virtual DescriptorCollection GetDescriptors() const = 0;
  virtual int NumVertices() const = 0;
  virtual ~MXMDVertexBuffer() {}
};

class MXMDFaceBuffer {
public:
  typedef std::unique_ptr<MXMDFaceBuffer> Ptr;

  virtual int GetNumIndices() const = 0;
  virtual const USVector *GetBuffer() const = 0;
  virtual ~MXMDFaceBuffer() {}
};

struct MXMDVertexWeight {
  Vector4 weights;
  UCVector4 boneids;
};

class MXMDGeomVertexWeightBuffer {
public:
  typedef std::unique_ptr<MXMDGeomVertexWeightBuffer> Ptr;

  virtual MXMDVertexWeight GetVertexWeight(int id) const = 0;
  virtual ~MXMDGeomVertexWeightBuffer() {}
};

class MXMDMorphTargets {
public:
  typedef std::unique_ptr<MXMDMorphTargets> Ptr;

  virtual int GetNumMorphs() const = 0;
  virtual int GetMorphNameID(int id) const = 0;
  virtual MXMDVertexBuffer::DescriptorCollection GetBaseMorph() const = 0;
  virtual MXMDVertexBuffer::DescriptorCollection
  GetDeltaMorph(int id) const = 0;
  virtual ~MXMDMorphTargets() {}
};

class MXMDGeomBuffers {
public:
  typedef std::unique_ptr<MXMDGeomBuffers> Ptr;

  virtual MXMDVertexBuffer::Ptr GetVertexBuffer(int id) const = 0;
  virtual int GetNumVertexBuffers() const = 0;
  virtual MXMDFaceBuffer::Ptr GetFaceBuffer(int id) const = 0;
  virtual int GetNumFaceBuffers() const = 0;
  virtual MXMDGeomVertexWeightBuffer::Ptr GetWeightsBuffer(int flags) const = 0;
  virtual MXMDMorphTargets::Ptr
  GetVertexBufferMorphTargets(int vertexBufferID) const = 0;

  virtual void SwapEndian(){};
  virtual ~MXMDGeomBuffers() {}
};

class MXMDTextures {
  template <class _Ty>
  int _ExtractAllTextures(const _Ty *outputFolder,
                          TextureConversionParams params) const;

public:
  typedef std::unique_ptr<MXMDTextures> Ptr;

  virtual int GetNumTextures() const = 0;
  virtual const char *GetTextureName(int id) const = 0;
  virtual int ExtractTexture(const wchar_t *outputFolder, int id,
                             TextureConversionParams params) const = 0;
  virtual int ExtractTexture(const char *outputFolder, int id,
                             TextureConversionParams params) const = 0;

  int ExtractAllTextures(const wchar_t *outputFolder,
                         TextureConversionParams params) const {
    return _ExtractAllTextures(outputFolder, params);
  }
  int ExtractAllTextures(const char *outputFolder,
                         TextureConversionParams params) const {
    return _ExtractAllTextures(outputFolder, params);
  }

  virtual void SwapEndian(){};
  virtual ~MXMDTextures() {}
};

class MXMDInstances {
public:
  typedef std::unique_ptr<MXMDInstances> Ptr;

  virtual int GetNumInstances() const = 0;
  virtual const MXMDTransformMatrix *GetTransform(int id) const = 0;
  virtual int GetStartingGroup(int id) const = 0;
  virtual int GetNumGroups(int id) const = 0;
  virtual int GetMeshGroup(int id) const = 0;

  virtual void SwapEndian(){};
  virtual ~MXMDInstances() {}
};

class MXMDShaders {
public:
  typedef std::unique_ptr<MXMDShaders> Ptr;

  virtual int GetNumShaders() const = 0;
  virtual void *GetShaderFile(int id) const = 0;

  virtual void SwapEndian(){};
  virtual ~MXMDShaders() {}
};

class MXMDExternalTextures {
public:
  typedef std::unique_ptr<MXMDExternalTextures> Ptr;

  virtual int GetNumTextures() const = 0;
  virtual int GetExTextureID(int id) const = 0;
  virtual int GetContainerID(int id) const = 0;

  virtual void SwapEndian(){};
  virtual ~MXMDExternalTextures() {}
};

class MXMDExternalResource {
public:
  virtual ~MXMDExternalResource() {}
};

class MXMD {
  static constexpr int ID_BIG = CompileFourCC("MXMD");
  static constexpr int ID = CompileFourCC("DMXM");
  union {
    char *masterBuffer;
    MXMDHeader *header;
  } data;
  MXMDExternalResource *externalResource;

  template <class _Ty0>
  // typedef wchar_t _Ty0;
  int _Load(const _Ty0 *fileName, bool suppressErrors);

public:
  MXMD() : data(), externalResource(nullptr) {}
  ~MXMD();

  int Load(const char *fileName, bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }
  int Load(const wchar_t *fileName, bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }
  MXMDModel::Ptr GetModel();
  MXMDMaterials::Ptr GetMaterials();
  MXMDGeomBuffers::Ptr GetGeometry(int groupID = 0);
  MXMDTextures::Ptr GetTextures();
  MXMDInstances::Ptr GetInstances();
  MXMDShaders::Ptr GetShaders();
  MXMDExternalTextures::Ptr GetExternalTextures();
};
