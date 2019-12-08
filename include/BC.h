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

#include "datas/vectors.hpp"

template <class C> union BCPointer {
  uint64 varPtr;
  C *ptr;
  char *cPtr;

  template <class _C = C>
  typename std::enable_if<!std::is_void<_C>::value, _C>::type &operator*() {
    return *ptr;
  }

  template <class _C = C>
  typename std::enable_if<std::is_void<_C>::value, void>::type operator*() {}

  template <class _C = C>
  typename std::enable_if<!std::is_void<_C>::value, _C>::type &
  operator[](size_t index) {
    return ptr[index];
  }

  template <class _C = C>
  typename std::enable_if<std::is_void<_C>::value, void>::type
  operator[](size_t index) {}

  C *operator->() { return ptr; }
};

struct BCHeader {
  int magic;
  short unk0, numBlocks;
  int fileSize, numPointers;
  BCPointer<void> dataOffset;
  BCPointer<uint64> fixupsOffset;

  BCHeader() = delete;
  BCHeader(const BCHeader &other) = delete;
  BCHeader &operator=(const BCHeader &other) = delete;

  void Fixup();
};

template <class C> struct BCArray {
  typedef C value_type;
  BCPointer<C> data;
  int count, unk;
};

struct BCBlock {
  int lookupID, ID;
};

struct BCSKEL : BCBlock {
  static constexpr int TYPE = CompileFourCC("SKEL");

  struct BoneTransform {
    Vector4 position, rotation, scale;
  };

  struct BoneName {
    const char *name;
    int _pad[ES_X64 ? 2 : 3];
  };

  struct BoneData {
    BCArray<void> unk0;
    BCPointer<void> null;
    BCPointer<const char> rootBoneName;
    BCArray<short> boneLinks;
    BCArray<BoneName> boneNames;
    BCArray<BoneTransform> boneTransforms;
    BCArray<void> unk1[6];
  };

  BCPointer<BoneData> boneData;
};

struct BCANIM : BCBlock {
  static constexpr int TYPE = CompileFourCC("ANIM");

  struct CubicCurve {
    float items[4];

    void Evaluate(float &out, float delta) const;
  };

  struct CubicFrame {
    float frame;
  };

  struct CubicVector3Frame : CubicFrame {
    CubicCurve elements[3];

    void Evaluate(Vector &out, float delta) const;
  };

  struct CubicQuatFrame : CubicFrame {
    CubicCurve elements[4];

    void Evaluate(Vector4 &out, float delta) const;
  };

  struct TransformFrame {
    Vector position;
    Vector4 rotation;
    Vector scale;
  };

  struct AnimationTrack {
    BCArray<CubicVector3Frame> position;
    BCArray<CubicQuatFrame> rotation;
    BCArray<CubicVector3Frame> scale;

    void GetPosition(float time, Vector &out, BCANIM *hdr) const;
    void GetRotation(float time, Vector4 &out, BCANIM *hdr) const;
    void GetScale(float time, Vector &out, BCANIM *hdr) const;
    void GetTransform(float time, TransformFrame &out, BCANIM *hdr) const;
  };

  struct AnimationData {
    BCPointer<void> null00;
    int unk00;
    BCPointer<void> null01;
    BCPointer<void> null02;
    BCPointer<short> boneTableOffset;
    int boneCount;
    BCPointer<void> null03;
  };

  BCPointer<AnimationData> animData;
  BCPointer<void> null00;
  int unk00;
  BCPointer<void> null01;
  BCPointer<const char> animationName;
  short unk01[2];
  float frameRate, frameTime;
  int frameCount;
  int unk02[3];
  BCPointer<void> null02;
  BCArray<AnimationTrack> tracks;
};

class BC {
  static constexpr int ID = CompileFourCC("BC\0\0");

  union {
    void *linked;
    char *masterBuffer;
    BCHeader *header;
  } data;
  bool linked;
  template <class _Ty0>
  // typedef wchar_t _Ty0;
  int _Load(const _Ty0 *fileName, bool suppressErrors);

public:
  BC() : data(), linked(false) {}
  ~BC();

  int Load(const char *fileName, bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }

  int Load(const wchar_t *fileName,
                          bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }

  int Link(void *file);

  template <class C> C *GetClass() {
    BCBlock *block = reinterpret_cast<BCBlock *>(data.header->dataOffset.ptr);

    if (block->ID != C::TYPE)
      return nullptr;
    return static_cast<C *>(block);
  }
};