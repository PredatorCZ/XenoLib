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
#include "datas/supercore.hpp"

struct SARFileEntry {
  int offset, size, unk0;
  char fileName[52];
};

struct SARHeader {
  int magic, fileSize, version, numFiles, entriesOffset, dataOffset, unk0, unk1;
  char mainPath[128];
};

class SAR {
  static constexpr int ID = CompileFourCC("1RAS");

  union {
    char *masterBuffer;
    SARHeader *header;
  } data;

  template <class _Ty0>
  // typedef wchar_t _Ty0;
  int _Load(const _Ty0 *fileName, bool suppressErrors);

  SARFileEntry *GetFileEntry(int id) const {
    return reinterpret_cast<SARFileEntry *>(data.masterBuffer +
                                            data.header->entriesOffset) +
           id;
  }

public:
  SAR() : data() {}
  ~SAR();
  int FileIndexFromExtension(const char *ext, int offset = 0);

  int Load(const char *fileName, bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }
  int Load(const wchar_t *fileName, bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }
  bool IsValid() const { return data.masterBuffer != nullptr; }
  int NumFiles() const { return data.header->numFiles; }
  void *GetFile(int id) const {
    return data.masterBuffer + GetFileEntry(id)->offset;
  }
  const char *GetFileName(int id) const { return GetFileEntry(id)->fileName; }
  int GetFileSize(int id) const { return GetFileEntry(id)->size; }
};
