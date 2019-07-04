/*	AddrLib is a part of GTX Extractor, ported and optimized into C languange
	Copyright(C) 2015-2018 AboodXD
	Copyright(C) 2018-2019 Lukas Cone

	This program is free software : you can redistribute it and /or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < http://www.gnu.org/licenses/>.
*/

#include "addrlib.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

unsigned int computeSurfaceThickness(GX2TileMode tileMode)
{
	switch (tileMode)
	{
	case GX2_TILE_MODE_1D_TILED_THICK:
	case GX2_TILE_MODE_2D_TILED_THICK:
	case GX2_TILE_MODE_2B_TILED_THICK:
	case GX2_TILE_MODE_3D_TILED_THICK:
	case GX2_TILE_MODE_3B_TILED_THICK:
		return 4;
	case GX2_TILE_MODE_LINEAR_SPECIAL:
	case GX2_TILE_MODE_LINEAR_SPECIAL2:
		return 8;
	default:
		return 1;
	}

	return 1;
}

unsigned int computePixelIndexWithinMicroTile(unsigned int x, unsigned int y, unsigned int bpp)
{
	switch (bpp)
	{
	case 0x8:
		return 32 * ((y & 4) >> 2) | 16 * (y & 1) | 8 * ((y & 2) >> 1) |
			4 * ((x & 4) >> 2) | 2 * ((x & 2) >> 1) | (x & 1);
	case 0x10:
		return 32 * ((y & 4) >> 2) | 16 * ((y & 2) >> 1) | 8 * (y & 1) |
			4 * ((x & 4) >> 2) | 2 * ((x & 2) >> 1) | (x & 1);
	case 0x20:
	case 0x60:
		return 32 * ((y & 4) >> 2) | 16 * ((y & 2) >> 1) | 8 * ((x & 4) >> 2) |
			4 * (y & 1) | 2 * ((x & 2) >> 1) | (x & 1);
	case 0x40:
		return 32 * ((y & 4) >> 2) | 16 * ((y & 2) >> 1) | 8 * ((x & 4) >> 2) |
			4 * ((x & 2) >> 1) | 2 * (y & 1) | (x & 1);
	case 0x80:
		return 32 * ((y & 4) >> 2) | 16 * ((y & 2) >> 1) | 8 * ((x & 4) >> 2) |
			4 * ((x & 2) >> 1) | 2 * (x & 1) | (y & 1);
	default:
		break;
	}

	return 32 * ((y & 4) >> 2) | 16 * ((y & 2) >> 1) | 8 * ((x & 4) >> 2) |
		4 * (y & 1) | 2 * ((x & 2) >> 1) | (x & 1);
}

unsigned int isThickMacroTiled(GX2TileMode tileMode)
{
	switch (tileMode)
	{

	case GX2_TILE_MODE_2D_TILED_THICK:
	case GX2_TILE_MODE_2B_TILED_THICK:
	case GX2_TILE_MODE_3D_TILED_THICK:
	case GX2_TILE_MODE_3B_TILED_THICK:
		return 1;
	default:
		return 0;
	}

	return 0;
}


unsigned int isBankSwappedTileMode(GX2TileMode tileMode)
{
	switch (tileMode)
	{
	case GX2_TILE_MODE_2B_TILED_THIN1:
	case GX2_TILE_MODE_2B_TILED_THIN2:
	case GX2_TILE_MODE_2B_TILED_THIN4:
	case GX2_TILE_MODE_2B_TILED_THICK:
	case GX2_TILE_MODE_3B_TILED_THIN1:
	case GX2_TILE_MODE_3B_TILED_THICK:
		return 1;
	default:
		return 0;
	}

	return 0;
}

unsigned int computeMacroTileAspectRatio(GX2TileMode tileMode)
{
	switch (tileMode)
	{
	case GX2_TILE_MODE_2D_TILED_THIN2:
	case GX2_TILE_MODE_2B_TILED_THIN2:
		return 2;
	case GX2_TILE_MODE_2D_TILED_THIN4:
	case GX2_TILE_MODE_2B_TILED_THIN4:
		return 4;
	default:
		return 1;
	}

	return 1;
}

unsigned int computeSurfaceBankSwappedWidth(GX2TileMode tileMode, unsigned int bpp, unsigned int pitch, unsigned int numSamples)
{
	if (!isBankSwappedTileMode(tileMode))
		return 0;

	const unsigned int bytesPerSample = 8 * bpp,
		slicesPerTile = bytesPerSample ? 1 : max(1, numSamples / (2048 / bytesPerSample));

	if (isThickMacroTiled(tileMode))
		numSamples = 4;

	const unsigned int bytesPerTileSlice = numSamples * bytesPerSample / slicesPerTile;
	const unsigned int factor = computeMacroTileAspectRatio(tileMode);
	const unsigned int swapTiles = max(1, 128 / bpp);
	const unsigned int swapWidth = swapTiles * 32;
	const unsigned int heightBytes = numSamples * factor * bpp * 2 / slicesPerTile;
	const unsigned int swapMax = 0x4000 / heightBytes;
	const unsigned int swapMin = 256 / bytesPerTileSlice;
	unsigned int bankSwapWidth = min(swapMax, max(swapMin, swapWidth));

	while (bankSwapWidth >= 2 * pitch)
		bankSwapWidth >>= 1;

	return bankSwapWidth;
}

AddrLibMicroTilePrecomp::AddrLibMicroTilePrecomp(unsigned int _bpp, unsigned int pitch, GX2TileMode tileMode) : bpp(_bpp)
{
	const int microTileThickness = tileMode == GX2_TILE_MODE_1D_TILED_THICK ? 4 : 1;
	microTileBytes = (64 * microTileThickness * _bpp + 7) / 8;
	microTilesPerRow = pitch >> 3;
}

unsigned int computeSurfaceAddrFromCoordMicroTiled(unsigned int x, unsigned int y, const AddrLibMicroTilePrecomp &precomp)
{
	const unsigned int microTileIndexX = x >> 3;
	const unsigned int microTileIndexY = y >> 3;

	const unsigned int microTileOffset = precomp.microTileBytes * (microTileIndexX + microTileIndexY * precomp.microTilesPerRow);
	const unsigned int pixelIndex = computePixelIndexWithinMicroTile(x, y, precomp.bpp);
	const unsigned int pixelOffset = (precomp.bpp * pixelIndex) >> 3;

	return pixelOffset + microTileOffset;
}


AddrLibMacroTilePrecomp::AddrLibMacroTilePrecomp(unsigned int _bpp, unsigned int pitch, unsigned int height,
	GX2TileMode tileMode, unsigned int pipeSwizzle, unsigned int bankSwizzle) : bpp(_bpp)
{
	microTileThickness = computeSurfaceThickness(tileMode);
	microTileBits = _bpp * (microTileThickness * 64);
	microTileBytes = (microTileBits + 7) / 8;

	microTileBits = _bpp * (microTileThickness * 64);
	microTileBytes = (microTileBits + 7) / 8;

	if (microTileBytes <= 2048)
	{
		numSamples = 1;
		sampleSlice = 0;
	}
	else
	{
		numSamples = 2048 / microTileBytes;
	}

	swizzle_ = pipeSwizzle + 2 * bankSwizzle;
	sliceBytes = (height * pitch * microTileThickness * _bpp * numSamples + 7) / 8;

	macroTileAspectRatio = computeMacroTileAspectRatio(tileMode);
	macroTilePitch = 32 / macroTileAspectRatio;
	macroTileHeight = 16 * macroTileAspectRatio;

	macroTilesPerRow = pitch / macroTilePitch;
	macroTileBytes = (numSamples * microTileThickness * _bpp * macroTileHeight * macroTilePitch + 7) / 8;

	bankSwapWidth = computeSurfaceBankSwappedWidth(tileMode, _bpp, pitch, 1);

	swappedBank = isBankSwappedTileMode(tileMode);
}

static const unsigned char bankSwapOrder[] = { 0, 1, 3, 2, 6, 7, 5, 4, 0, 0 };

unsigned int computeSurfaceAddrFromCoordMacroTiled(unsigned int x, unsigned int y, const AddrLibMacroTilePrecomp &precomp)
{
	unsigned int elemOffset = precomp.bpp * computePixelIndexWithinMicroTile(x, y, precomp.bpp);

	if (precomp.microTileBytes > 2048)
	{
		precomp.sampleSlice = elemOffset / (precomp.microTileBits);
		elemOffset %= precomp.microTileBits;
	}

	elemOffset = (elemOffset + 7) / 8;

	unsigned int pipe = ((y >> 3) ^ (x >> 3)) & 1;
	unsigned int bank = (((y >> 5) ^ (x >> 3)) & 1) | (2 * (((y >> 4) ^ (x >> 4)) & 1));

	const unsigned int bankPipe = ((pipe + 2 * bank) ^ (6 * precomp.sampleSlice ^ precomp.swizzle_)) % 8;

	pipe = bankPipe % 2;
	bank = bankPipe / 2;

	const unsigned int sliceOffset = precomp.sliceBytes * (precomp.sampleSlice / precomp.microTileThickness);

	const unsigned int macroTileIndexX = x / precomp.macroTilePitch;
	const unsigned int macroTileIndexY = y / precomp.macroTileHeight;
	const unsigned int macroTileOffset = (macroTileIndexX + precomp.macroTilesPerRow * macroTileIndexY) * precomp.macroTileBytes;

	if (precomp.swappedBank)
	{
		const unsigned int swapIndex = precomp.macroTilePitch * macroTileIndexX / precomp.bankSwapWidth;
		bank ^= bankSwapOrder[swapIndex & 3];
	}

	const unsigned int totalOffset = elemOffset + ((macroTileOffset + sliceOffset) >> 3);
	return (bank << 9) | (pipe << 8) | (255 & totalOffset) | ((totalOffset & -256) << 3);
}
