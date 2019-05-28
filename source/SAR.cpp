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

#include "SAR.h"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"

template<class _Ty0>
int SAR::_Load(const _Ty0 *fileName, bool suppressErrors)
{
	BinReader rd(fileName);

	if (!rd.IsValid())
	{
		if (!suppressErrors)
		{
			printerror("[SAR] Cannot load file: ", << fileName);
		}

		return 1;
	}

	SARHeader hdr;
	rd.Read(hdr);

	if (hdr.magic != ID)
	{
		if (!suppressErrors)
		{
			printerror("[SAR] Invalid header.");
		}

		return 2;
	}

	if (hdr.version > 500)
	{
		if (!suppressErrors)
		{
			printerror("[SAR] Unexpected version.");
		}

		return 3;
	}

	rd.Seek(0);

	data.masterBuffer = static_cast<char *>(malloc(hdr.fileSize));
	rd.ReadBuffer(data.masterBuffer, hdr.fileSize);

	return 0;
}

template int SAR::_Load(const char *fileName, bool suppressErrors);
template int SAR::_Load(const wchar_t *fileName, bool suppressErrors);

int SAR::FileIndexFromExtension(const char *ext, int offset)
{
	const int numFiles = NumFiles();

	for (int f = offset; f < numFiles; f++)
	{
		std::string fleName = GetFileName(f);
		size_t dotPos = fleName.find_last_of('.');

		if (dotPos == fleName.npos)
			continue;

		if (!fleName.substr(dotPos).compare(ext))
			return f;
	}

	return -1;
}

SAR::~SAR()
{
	if (data.masterBuffer)
		free(data.masterBuffer);
}
