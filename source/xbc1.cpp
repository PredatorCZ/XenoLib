/*  Xenoblade Engine Format Library
    Copyright(C) 2017-2023 Lukas Cone

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

#include "xenolib/xbc1.hpp"
#include "spike/except.hpp"
#include "zlib.h"
#define ZSTD_DISABLE_DEPRECATE_WARNINGS
#include "zstd.h"

namespace {
enum class CompType {
  Zlib = 1,
  Zstd = 3,
};

struct xbc1 {
  static constexpr int ID = CompileFourCC("xbc1");

  uint32 id;
  CompType compressionType;
  uint32 uncompressedSize;
  uint32 compressedSize;
  uint32 hash;
  char name[28];
};
} // namespace

std::string DecompressXBC1(const char *data) {
  auto *hdr = reinterpret_cast<const xbc1 *>(data);

  if (hdr->id != xbc1::ID) {
    throw es::InvalidHeaderError(hdr->id);
  }

  std::string retval;
  retval.resize(hdr->uncompressedSize);
  uLongf dataWritten = retval.size();

  if (hdr->compressionType == CompType::Zlib) {
    if (int status = uncompress(
            reinterpret_cast<Bytef *>(retval.data()), &dataWritten,
            reinterpret_cast<const Bytef *>(hdr + 1), hdr->compressedSize);
        status != Z_OK) [[unlikely]] {
      if (status == Z_MEM_ERROR) {
        throw std::runtime_error("Zlib, not enough memory");
      } else if (status == Z_DATA_ERROR) [[likely]] {
        throw std::runtime_error("Zlib, data is corrupted");
      } else if (status == Z_BUF_ERROR) {
        throw std::runtime_error("Zlib, output buffer is not big enough");
      } else [[unlikely]] {
        throw std::runtime_error("Zlib, decompression failed");
      }
    }
  } else if (hdr->compressionType == CompType::Zstd) {
    size_t status = ZSTD_decompress(retval.data(), dataWritten, hdr + 1,
                                    hdr->compressedSize);

    if (ZSTD_isError(status)) {
      throw std::runtime_error("ZSTD, decompression failed: " +
                               std::string(ZSTD_getErrorName(status)));
    }

  } else {
    throw std::runtime_error("invalid compression type: " +
                             std::to_string(uint32(hdr->compressionType)));
  }

  return retval;
}
