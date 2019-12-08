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

#include "BC.h"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"

void BCHeader::Fixup() {
  if (!numPointers)
    return;

  char *thisAddr = reinterpret_cast<char *>(this);

  fixupsOffset.cPtr = thisAddr + fixupsOffset.varPtr;
  dataOffset.cPtr = thisAddr + dataOffset.varPtr;

  for (int f = 0; f < numPointers; f++)
    *reinterpret_cast<esIntPtr *>(thisAddr + fixupsOffset[f]) +=
        reinterpret_cast<esIntPtr>(thisAddr);

  numPointers = 0;
}

template <class _Ty0> int BC::_Load(const _Ty0 *fileName, bool suppressErrors) {
  BinReader rd(fileName);

  if (!rd.IsValid()) {
    if (!suppressErrors) {
      printerror("[BC] Cannot load file: ", << fileName);
    }

    return 1;
  }

  int magic;
  rd.Read(magic);

  if (magic != ID) {
    if (!suppressErrors) {
      printerror("[BC] Invalid header.");
    }

    return 2;
  }

  rd.Seek(0);

  const size_t fileSize = rd.GetSize();

  data.masterBuffer = static_cast<char *>(malloc(fileSize));
  rd.ReadBuffer(data.masterBuffer, fileSize);
  data.header->Fixup();

  return 0;
}

template int BC::_Load(const char *fileName, bool suppressErrors);
template int BC::_Load(const wchar_t *fileName, bool suppressErrors);

int BC::Link(void *file) {
  data.linked = file;

  if (data.header->magic != ID) {
    printerror("[BC] Invalid header.");
    return 1;
  }

  data.header->Fixup();
  linked = true;
  return 0;
}

BC::~BC() {
  if (data.masterBuffer && !linked)
    free(data.masterBuffer);
}

ES_INLINE void BCANIM::CubicCurve::Evaluate(float &out, float delta) const {
  out = (delta * delta * delta) * items[0] + (delta * delta) * items[1] +
        delta * items[2] + items[3];
}

ES_INLINE void BCANIM::CubicVector3Frame::Evaluate(Vector &out,
                                                   float delta) const {
  elements[0].Evaluate(out[0], delta);
  elements[1].Evaluate(out[1], delta);
  elements[2].Evaluate(out[2], delta);
}

ES_INLINE void BCANIM::CubicQuatFrame::Evaluate(Vector4 &out,
                                                float delta) const {
  elements[0].Evaluate(out[0], delta);
  elements[1].Evaluate(out[1], delta);
  elements[2].Evaluate(out[2], delta);
  elements[3].Evaluate(out[3], delta);
}

template <class T, class V>
ES_INLINE void GetEvalValue(const T &frames, float time, BCANIM *hdr, V &out) {
  const typename T::value_type *lastFrame = frames.data.ptr;
  const typename T::value_type *endFrame = frames.data.ptr + frames.count - 1;
  float delta = 0.0f;
  float requiredFrame = time / hdr->frameTime;

  if (frames.count > 1) {
    for (int i = 0; i < frames.count; i++, lastFrame++) {
      if (lastFrame->frame > requiredFrame) {
        lastFrame--;
        break;
      }
    }

    if (lastFrame > endFrame)
      lastFrame = endFrame;

    if (requiredFrame > static_cast<float>(hdr->frameCount))
      requiredFrame = static_cast<float>(hdr->frameCount);

    delta = requiredFrame - lastFrame->frame;
  }

  lastFrame->Evaluate(out, delta);
}

ES_INLINE void BCANIM::AnimationTrack::GetPosition(float time, Vector &out,
                                                   BCANIM *hdr) const {
  GetEvalValue(position, time, hdr, out);
}

ES_INLINE void BCANIM::AnimationTrack::GetRotation(float time, Vector4 &out,
                                                   BCANIM *hdr) const {
  GetEvalValue(rotation, time, hdr, out);
}

ES_INLINE void BCANIM::AnimationTrack::GetScale(float time, Vector &out,
                                                BCANIM *hdr) const {
  GetEvalValue(scale, time, hdr, out);
}

void BCANIM::AnimationTrack::GetTransform(float time, TransformFrame &out,
                                          BCANIM *hdr) const {
  GetPosition(time, out.position, hdr);
  GetRotation(time, out.rotation, hdr);
  GetScale(time, out.scale, hdr);
}