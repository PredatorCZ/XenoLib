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

#include <fstream>
#include "formats/DDS.hpp"
#include "formats/BlockDecoder.inl"
#include "datas/endian.hpp"
#include "datas/supercore.hpp"
#include "datas/MasterPrinter.hpp"
#include "datas/esstring.h"
#include "XenoLibAPI.h"
#include "png.h"
#include "addrlib.h"
#include "addrlib.inl"

void WritePng(std::ofstream *stream, const char *buffer, int size, int width, int height, int colorType, int bpr, bool flipRB);

struct MTXT
{
	static const int ID = CompileFourCC("MTXT");
	static const int IDs = CompileFourCC("TXTM");

	int swizzle,
		dimension,
		width,
		height,
		depth,
		nomips,
		type,
		size,
		aamode;
	GX2TileMode tiling;
	int unk,
		alignment,
		pitch,
		null[13],
		version,
		magic;
};

struct ConvertMTXT_Out
{
	int result;
	const char *outBuffer;
	int outBufferSize;
	int pngColorFormat, upc;
	bool flipRB;
};

ConvertMTXT_Out ConvertMTXT(const char *buffer, int size, TextureConversionParams &params, DDS &ddsFile)
{
	MTXT header = *reinterpret_cast<const MTXT*>(buffer + size - sizeof(MTXT));
	constexpr int numHeaderItems = sizeof(MTXT) / 4;
	ConvertMTXT_Out retVal = {};

	if (header.magic == header.ID)
	{
		for (int i = 0; i < numHeaderItems; i++)
			FByteswapper(*(reinterpret_cast<int *>(&header) + i));
	}
	else if (header.magic != header.IDs)
	{
		printerror("[MTXT] Invalid header");
		retVal.result = 1;
		return retVal;
	}

	ddsFile.width = header.width;
	ddsFile.height = header.height;
	
	int bpp = 0, ppb = 1, upc = 4, pngColorFormat = 0; // bits per pixel, pixels per block, uncompressed pixels per color

	void(*DecompFunc)(const char *, char *, int, int, int) = nullptr;
	bool computeBlueChan = false, flipRB = true, scanAlpha = false;

	switch (header.type)
	{
	case GX2_SURFACE_FORMAT_T_BC1_UNORM:
	case GX2_SURFACE_FORMAT_T_BC1_SRGB:
		ddsFile = DDSFormat_DXT1;
		bpp = 64;
		ppb = 4;
		DecompFunc = &DecodeBC1BlockA;
		scanAlpha = true;
		break;
	case GX2_SURFACE_FORMAT_T_BC2_UNORM:
	case GX2_SURFACE_FORMAT_T_BC2_SRGB:
		ddsFile = DDSFormat_DXT4;
		bpp = 64;
		ppb = 4;
		DecompFunc = &DecodeBC2Block;
		scanAlpha = true;
		break;
	case GX2_SURFACE_FORMAT_T_BC3_UNORM:
	case GX2_SURFACE_FORMAT_T_BC3_SRGB:
		ddsFile = DDSFormat_DXT5;
		bpp = 128;
		ppb = 4;
		DecompFunc = &DecodeBC3Block;
		break;
	case GX2_SURFACE_FORMAT_T_BC4_UNORM:
	case GX2_SURFACE_FORMAT_T_BC4_SNORM:
		ddsFile = DDSFormat_ATI1;
		bpp = 64;
		ppb = 4;
		upc = 1;
		DecompFunc = &DecodeBC4Block;
		break;
	case GX2_SURFACE_FORMAT_T_BC5_UNORM:
	case GX2_SURFACE_FORMAT_T_BC5_SNORM:
		ddsFile = DDSFormat_ATI2;
		bpp = 128;
		ppb = 4;
		upc = params.allowBC5ZChan ? 3 : 2;
		DecompFunc = params.allowBC5ZChan ? &DecodeBC5Block : &DecodeBC5BlockGA;
		computeBlueChan = params.allowBC5ZChan;
		break;
	case GX2_SURFACE_FORMAT_TC_R8_UNORM:
		ddsFile = DDSFormat_L8;
		bpp = 8;
		upc = 1;
		break;

	/*case GX2_SURFACE_FORMAT_TC_R4_G4_B4_A4_UNORM:
		hdr.ddspf = DDS::DDSPF_A4R4G4B4;
		bpp = 16;
		flipRB = false;
		break;
	case GX2_SURFACE_FORMAT_TC_R5_G5_B5_A1_UNORM:
		hdr.ddspf = DDS::DDSPF_A1R5G5B5;
		bpp = 16;
		flipRB = false;
		break;
	case GX2_SURFACE_FORMAT_TCS_R5_G6_B5_UNORM:
		hdr.ddspf = DDS::DDSPF_R5G6B5;
		bpp = 16;
		flipRB = false;
		break;*/
	case GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM:
		ddsFile = DDSFormat_A8R8G8B8;
		bpp = 32;
		flipRB = false;
		scanAlpha = true;
		break;


		/*
		GX2_SURFACE_FORMAT_TC_R32_G32_B32_A32_FLOAT
		GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_UNORM
		GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_FLOAT
		GX2_SURFACE_FORMAT_TCS_R10_G10_B10_A2_UNORM
		*/
	default:
		printerror("[MTXT] Unhandled texture format: ", << header.type);
		retVal.result = 2;
		return retVal;
		break;
	}

	params.uncompress = params.uncompress && (header.type == GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM || DecompFunc);

	if (!params.uncompress)
		DecompFunc = nullptr;
	else
	{
		switch (upc)
		{
		case 1:
			pngColorFormat = PNG_COLOR_TYPE_GRAY;
			break;
		case 2:
			pngColorFormat = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;
		case 3:
			pngColorFormat = PNG_COLOR_TYPE_RGB;
			break;
		case 4:
			pngColorFormat = PNG_COLOR_TYPE_RGBA;
			break;
		default:
			break;
		}
	}

	const int width = header.width / ppb;
	const int height = header.height / ppb;
	const int Bpp = bpp / 8;
	const int surfaceSize = width * height * Bpp;
	char *deswbuffer = nullptr;
	int decSurfaceSize = params.uncompress ? (header.width *  header.height * upc) :  size;

	if (header.tiling > 1)
	{
		deswbuffer = static_cast<char *>(malloc(decSurfaceSize));
		memset(deswbuffer, 0xff, decSurfaceSize);
		const int pipeSwizzle = (header.swizzle >> 8) & 1;
		const int bankSwizzle = (header.swizzle >> 9) & 3;

		if (header.tiling < 4)
		{
			const AddrLibMicroTilePrecomp precomp(bpp, header.pitch, header.tiling);

			for (int w = 0; w < width; w++)
				for (int h = 0; h < height; h++)
				{
					int address = computeSurfaceAddrFromCoordMicroTiled(w, h, precomp);

					if (DecompFunc)
						DecompFunc(buffer + address, deswbuffer, w, h, width);
					else
						for (int p = 0; p < Bpp; p++)
							*(deswbuffer + ((h * width + w) * Bpp) + p) = *(buffer + address + p);
				}
		}
		else
		{
			const AddrLibMacroTilePrecomp precomp(bpp, header.pitch, height, header.tiling, pipeSwizzle, bankSwizzle);

			for (int w = 0; w < width; w++)
				for (int h = 0; h < height; h++)
				{
					int address = computeSurfaceAddrFromCoordMacroTiled(w, h, precomp);

					if (DecompFunc)
						DecompFunc(buffer + address, deswbuffer, w, h, width);
					else
						for (int p = 0; p < Bpp; p++)
							*(deswbuffer + ((h * width + w) * Bpp) + p) = *(buffer + address + p);
				}
		}
	}
	else if (params.uncompress && DecompFunc)
	{
		deswbuffer = static_cast<char *>(malloc(decSurfaceSize));
		memset(deswbuffer, 0xff, decSurfaceSize);
		for (int w = 0; w < width; w++)
			for (int h = 0; h < height; h++)
				DecompFunc(buffer + ((h * width + w) * Bpp), deswbuffer, w, h, width);

	}
	else if (!params.uncompress)
	{
		retVal.outBuffer = buffer;
		retVal.outBufferSize = surfaceSize;
	}

	if (params.uncompress)
	{
		if (computeBlueChan)
			ComputeBC5Blue(deswbuffer, decSurfaceSize);
		if (scanAlpha)
		{
			for (int p = 3; p < decSurfaceSize; p += 4)
				if (*reinterpret_cast<uchar*>(deswbuffer + p) < 255)
				{
					scanAlpha = false;
					break;
				}
			if (scanAlpha)
			{
				const int newSurfaceSize = header.width * header.height * 3;
				char *newBuffer = static_cast<char*>(malloc(newSurfaceSize));

				for (int p = 0, n = 0; p < decSurfaceSize; p += 4, n += 3)
					*reinterpret_cast<UCVector*>(newBuffer + n) = *reinterpret_cast<UCVector*>(deswbuffer + p);

				free(deswbuffer);
				deswbuffer = newBuffer;
				decSurfaceSize = newSurfaceSize;
				pngColorFormat = PNG_COLOR_TYPE_RGB;
				upc = 3;
			}
		}

		retVal.outBuffer = deswbuffer ? deswbuffer : buffer;
		retVal.outBufferSize = deswbuffer ? decSurfaceSize : surfaceSize;
		retVal.flipRB = flipRB;
		retVal.upc = upc;
		retVal.pngColorFormat = pngColorFormat;
	}
	else if (deswbuffer)
	{
		retVal.outBuffer = deswbuffer;
		retVal.outBufferSize = decSurfaceSize;
	}
	
	return retVal;
}

template<class _Ty>
int _ConvertMTXT(const char *buffer, int size, const _Ty *_path, TextureConversionParams params)
{
	DDS ddsFile = {};
	ConvertMTXT_Out result = ConvertMTXT(buffer, size, params, ddsFile);

	if (result.result)
		return result.result;

	UniString<_Ty> path = _path;

	if (!params.uncompress)
		path.append(esString(".dds"));
	else
		path.append(esString(".png"));

	std::ofstream ofs(path, std::ios::binary | std::ios::out);

	if (ofs.fail())
	{
		printerror("[MTXT] Cannot create file at \"", << path.c_str() << " \", make sure you can write there or path is valid.");
		return 3;
	}

	if (!params.uncompress)
	{
		ofs.write(reinterpret_cast<const char *>(&ddsFile), DDS::LEGACY_SIZE);
		ofs.write(result.outBuffer, result.outBufferSize);
	}
	else
	{
		WritePng(&ofs, result.outBuffer, result.outBufferSize, ddsFile.width, ddsFile.height, result.pngColorFormat, result.upc, result.flipRB);
	}

	if (result.outBuffer != buffer)
		free(const_cast<char*>(result.outBuffer));

	return 0;
}

int ConvertMTXT(const char *buffer, int size, const char *path, TextureConversionParams params)
{
	return _ConvertMTXT(buffer, size, path, params);
}

int ConvertMTXT(const char *buffer, int size, const wchar_t *path, TextureConversionParams params)
{
	return _ConvertMTXT(buffer, size, path, params);
}