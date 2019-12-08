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

#include "DRSM.h"
#include "datas/MultiThread.hpp"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"
#include "zlib.h"

struct xbc1Queue {
  int queue;
  int queueEnd;
  DRSMResourceStream *streams;
  char *mainBuffer;
  std::vector<char *> *resources;

  typedef void return_type;

  xbc1Queue() : queue(0) {}

  return_type RetreiveItem() {
    char *resBuffer = mainBuffer + streams[queue].offset;
    resources->at(queue) = ExtractXBC(resBuffer);
  }

  operator bool() { return queue < queueEnd; }
  void operator++(int) { queue++; }
  int NumQueues() const { return queueEnd; }
};

template <class _Ty0>
int DRSM::_Load(const _Ty0 *fileName, bool suppressErrors) {
  BinReader rd(fileName);

  if (!rd.IsValid()) {
    if (!suppressErrors) {
      printerror("[DRSM] Cannot open file.");
    }

    return 1;
  }

  DRSMHeader hdr;
  rd.Read(hdr);

  if (hdr.magic != ID) {
    if (!suppressErrors) {
      printerror("[DRSM] Invalid header.");
    }

    return 2;
  }

  data.masterBuffer = static_cast<char *>(malloc(hdr.dataSize));
  rd.ReadBuffer(data.masterBuffer, hdr.dataSize);

  const size_t bufferDelta = rd.Tell();
  const int resBufferSize = static_cast<int>(rd.GetSize() - bufferDelta);

  char *resBuffer = static_cast<char *>(malloc(resBufferSize));
  rd.ReadBuffer(resBuffer, resBufferSize);

  xbc1Queue resQue;
  resQue.queueEnd = data.header->numFiles;
  resQue.streams = data.header->Resources();
  resQue.mainBuffer = resBuffer - bufferDelta;
  resources.resize(resQue.queueEnd);
  resQue.resources = &resources;

  RunThreadedQueue(resQue);

  free(resBuffer);
  return 0;
}

template int DRSM::_Load(const char *fileName, bool suppressErrors);
template int DRSM::_Load(const wchar_t *fileName, bool suppressErrors);

const char *DRSM::GetTextureName(int id) const {
  DRSMTextureItem *cTex = data.header->TextureTable()->Textures() + id;
  return data.header->TextureTable()->TextureName(cTex);
}

DRSM::~DRSM() {
  if (data.masterBuffer)
    free(data.masterBuffer);

  for (auto &r : resources)
    free(r);
}

template <class _Ty0>
int DRSM::_ExtractTexture(const _Ty0 *outputFolder, int id,
                          TextureConversionParams params) const {
  DRSMTextureItem *cTex = data.header->TextureTable()->Textures() + id;
  const char *textureName = data.header->TextureTable()->TextureName(cTex);

  short *highIDs = data.header->IDs();
  int highMipID = -1;

  for (int i = 0; i < data.header->IDTableCount; i++)
    if (highIDs[i] == id) {
      highMipID = i;
      break;
    }

  DRSMResourceItem *resItems = data.header->ResourceItems();
  const UniString<_Ty0> outputName =
      outputFolder + esStringConvert<_Ty0>(textureName);
  char *hrMipBuffer = nullptr, *midMipBuffer = nullptr;
  int hrMipBufferSize = 0, midMipBufferSize = 0;

  if (highMipID > -1) {
    DRSMResourceItem &midMip = resItems[highMipID + 3];
    hrMipBuffer = resources[highMipID + 2];
    midMipBuffer = resources[1] + midMip.localOffset;
    midMipBufferSize = midMip.localSize;
    hrMipBufferSize = data.header->Resources()[highMipID + 2].uncompressedSize;
  } else {
    midMipBuffer = resources[0] + resItems[2].localOffset + cTex->cachedOffset;
    midMipBufferSize = cTex->cachedSize;
  }

  if (ConvertLBIM(midMipBuffer, midMipBufferSize, outputName.c_str(), params,
                  hrMipBuffer, hrMipBufferSize)) {
    printerror("[LBIM] Texture: ", << id << " could not be created.");
    return 1;
  }

  return 0;
}

template int DRSM::_ExtractTexture(const char *outputFolder, int id,
                                   TextureConversionParams params) const;
template int DRSM::_ExtractTexture(const wchar_t *outputFolder, int id,
                                   TextureConversionParams params) const;

struct xbc1 {
  static const int ID = CompileFourCC("xbc1");

  int magic, numFiles, uncompSize, size, hash;
  char name[28];
};

char *ExtractXBC(const char *buffer) {
  const xbc1 *hdr = reinterpret_cast<const xbc1 *>(buffer);

  if (hdr->magic != xbc1::ID) {
    printerror("[XBC1] Invalid header.");
    return nullptr;
  }

  Bytef *comBuffer =
      const_cast<Bytef *>(reinterpret_cast<const Bytef *>(hdr + 1));
  Bytef *uncompBuffer = static_cast<Bytef *>(malloc(hdr->uncompSize));
  z_stream infstream;
  infstream.zalloc = Z_NULL;
  infstream.zfree = Z_NULL;
  infstream.opaque = Z_NULL;
  infstream.avail_in = hdr->size;
  infstream.next_in = comBuffer;
  infstream.avail_out = hdr->uncompSize;
  infstream.next_out = uncompBuffer;
  inflateInit(&infstream);
  inflate(&infstream, Z_FINISH);
  inflateEnd(&infstream);
  return reinterpret_cast<char *>(uncompBuffer);
}
