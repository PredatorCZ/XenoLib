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
#include "XenoLibAPI.h"
#include "datas/masterprinter.hpp"
#include "datas/esstring.h"
#include "formats/DDS.hpp"
#include "formats/BlockDecoder.inl"
#include "png.h"

void WritePng(std::ofstream *stream, const char *buffer, int size, int width, int height, int colorType, int bpr, bool flipRB);

struct LBIM
{
	static const int ID = CompileFourCC("LBIM");

	int datasize,
		headersize,
		width,
		height,
		unk0[2],
		format,
		unk1,
		version,
		magic;
};

enum
{
	LBIM_BC3_UNORM = 68,
	LBIM_BC5_UNORM = 75,
	LBIM_BC1_UNORM = 66,
	LBIM_BC4_UNORM = 73,
	LBIM_BC2_UNORM = 67,
	LBIM_R8_G8_B8_A8_UNORM = 37
}LBIMFORMAT;

int NumLeadingZeroes(int Value)
{
	int numZeros = 0;
	for (; (((Value >> numZeros) & 1) == 0); numZeros++) {}
	return numZeros;
}

struct ConvertLBIM_Out
{
	int result;
	char *outBuffer;
	int outBufferSize;
	int pngColorFormat, upc;
	bool flipRB;
};

ConvertLBIM_Out ConvertLBIM(const char *buffer, int size, TextureConversionParams &params, const char *exBuffer, int exBuffSize, DDS &ddsFile)
{
	LBIM header = *reinterpret_cast<const LBIM*>(buffer + size - sizeof(LBIM));
	ConvertLBIM_Out retVal = {};

	if (header.magic != LBIM::ID)
	{
		printerror("[LBIM] Invalid header");
		retVal.result = 1;
		return retVal;
	}
	
	if (exBuffSize > 0)
	{
		header.width *= 2;
		header.height *= 2;
	}
	
	ddsFile.width = header.width;
	ddsFile.height = header.height;

	int bpp = 0, ppb = 1, upc = 4, pngColorFormat = 0; // bytes per pixel, pixels per block, uncompressed pixels per color
	void (*DecompFunc)(const char *, char *, int, int, int) = nullptr;
	bool computeBlueChan = false, flipRB = true, scanAlpha = false;

	switch (header.format) 
	{
	case LBIM_BC1_UNORM:
		ddsFile = DDSFormat_DXT1;
		bpp = 8;
		ppb = 4;
		DecompFunc = &DecodeBC1Block;
		//scanAlpha = true;
		upc = 3;
		break;
	case LBIM_BC2_UNORM:
		ddsFile = DDSFormat_DXT4;
		bpp = 16;
		ppb = 4;
		DecompFunc = &DecodeBC2Block;
		scanAlpha = true;
		break;
	case LBIM_BC3_UNORM:
		ddsFile = DDSFormat_DXT5;
		bpp = 16;
		ppb = 4;
		DecompFunc = &DecodeBC3Block;
		break;
	case LBIM_BC4_UNORM:
		ddsFile = DDSFormat_ATI1;
		bpp = 8;
		ppb = 4;
		upc = 1;
		DecompFunc = &DecodeBC4Block;
		break;
	case LBIM_BC5_UNORM:
		ddsFile = DDSFormat_ATI2;
		bpp = 16;
		ppb = 4;
		upc = params.allowBC5ZChan ? 3 : 2;
		DecompFunc = params.allowBC5ZChan ? &DecodeBC5Block : &DecodeBC5BlockGA;
		computeBlueChan = params.allowBC5ZChan;
		break;
	case LBIM_R8_G8_B8_A8_UNORM:
		ddsFile = DDS_PixelFormat({ DDS_PixelFormat::PFFlags_RGB, DDS_PixelFormat::PFFlags_AlphaPixels }, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
		bpp = 4;
		flipRB = false;
		scanAlpha = true;
		break;
	default:
		printerror("[LBIM] Unhandled texture format: ", << header.format);
		retVal.result = 2;
		return retVal;
		break;
	}

	params.uncompress = params.uncompress && (header.format == LBIM_R8_G8_B8_A8_UNORM || DecompFunc);

	if (params.uncompress)
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
	else	
		DecompFunc = nullptr;
	
	const char *baseBuffer = exBuffSize > 0 ? exBuffer : buffer;
	int decSurfaceSize = (header.width *  header.height * (params.uncompress ? upc * 8 : ddsFile.bpp)) / 8;

	char *deswbuffer = static_cast<char*>(malloc(decSurfaceSize));
	const int width = header.width / ppb;
	const int height = header.height / ppb;
	const int surfaceSize = width*height*bpp;
	const int bppShift = NumLeadingZeroes(bpp);
	const int lineShift = NumLeadingZeroes(width * bpp);

	int xBitsShift = 3;
	for (int i = 0; i < 4; i++)
		if (((height)-1) & (8 << i))
			xBitsShift++;

	for (int w = 0; w < width; w++)
		for (int h = 0; h < height; h++)
		{
			
			//12 11 10  9  8  7  6  5  4  3  2  1  0
			// y  y  y  y  x  y  y  x  y  x  x  x  x
		// X   -  -  -  -  5  -  -  4  -  3  2  1  0
		// Y   6  5  4  3  -  2  1  -  0  -  -  -  -

			/*
			0: 1
			1: 3
			2: 8
			3: 15
			4: 31
			5: 63
			6: 127
			7: 255
			8: 511
			9: 1023
			10:2047
			11:4095
			12:8191
			13:16384
			*/

			const int _X = w << bppShift;
			int address = (h & 0xff80) << lineShift		// bits xEnd - yEnd		
				
				| ((h & 0x78) << 6) //bits 9 - 12
				
				| ((h & 6) << 5) //bits 6,7

				| ((h & 1) << 4) //good bit 4

				| ((_X & 0xffc0) << xBitsShift)  //bits 13 - xEnd

				| ((_X & 0x20) << 3) //bit 8

				| ((_X & 0x10) << 1) //good bit 5
				
				| (_X & 0xf) //good, 0-3 bits
				;

			if (address + bpp > surfaceSize)
				continue;

			if (DecompFunc)
				DecompFunc(baseBuffer + address, deswbuffer, w, h, width);
			else
				for (int p = 0; p < bpp; p++)
					*(deswbuffer + ((h * width + w) * bpp) + p) = *(baseBuffer + address + p);
		}

	if (params.uncompress)
	{
		if (computeBlueChan)
			ComputeBC5Blue(deswbuffer, decSurfaceSize);

		if (scanAlpha)
		{
			for (int p = 3; p < decSurfaceSize; p += 4)
				if (*reinterpret_cast<unsigned char*>(deswbuffer + p) < 255)
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

		retVal.flipRB = flipRB;
		retVal.upc = upc;
		retVal.pngColorFormat = pngColorFormat;
	}

	retVal.outBuffer = deswbuffer;
	retVal.outBufferSize = decSurfaceSize;
	
	return retVal;
}

template<class _Ty>
int _ConvertLBIM(const char *buffer, int size, const _Ty *_path, TextureConversionParams params, const char *exBuffer, int exBuffSize)
{
	DDS ddsFile = {};
	ConvertLBIM_Out result = ConvertLBIM(buffer, size, params, exBuffer, exBuffSize, ddsFile);

	if (result.result)
		return result.result;

	UniString<_Ty> path = _path;

	if (!params.uncompress)
		path.append(esString(".dds"));
	else
		path.append(esString(".png"));

	std::ofstream ofs(esString(path), std::ios::binary | std::ios::out);

	if (ofs.fail())
	{
		printerror("[LBIM] Cannot create file at \"", << path.c_str() << " \", make sure you can write there or path is valid.");
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

	free(result.outBuffer);

	return 0;
}

int ConvertLBIM(const char *buffer, int size, const char *path, TextureConversionParams params, const char *exBuffer, int exBuffSize)
{
	return _ConvertLBIM(buffer, size, path, params, exBuffer, exBuffSize);
}

int ConvertLBIM(const char *buffer, int size, const wchar_t *path, TextureConversionParams params, const char *exBuffer, int exBuffSize)
{
	return _ConvertLBIM(buffer, size, path, params, exBuffer, exBuffSize);
}