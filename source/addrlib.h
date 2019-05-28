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

#pragma once

enum GX2TileMode
{
	GX2_TILE_MODE_DEFAULT = 0, // driver will choose best mode
	GX2_TILE_MODE_LINEAR_ALIGNED, // supported by HW, but not fast
	GX2_TILE_MODE_1D_TILED_THIN1,
	GX2_TILE_MODE_1D_TILED_THICK,
	GX2_TILE_MODE_2D_TILED_THIN1, // (a typical default, but not always)
	GX2_TILE_MODE_2D_TILED_THIN2,
	GX2_TILE_MODE_2D_TILED_THIN4,
	GX2_TILE_MODE_2D_TILED_THICK,
	GX2_TILE_MODE_2B_TILED_THIN1,
	GX2_TILE_MODE_2B_TILED_THIN2,
	GX2_TILE_MODE_2B_TILED_THIN4,
	GX2_TILE_MODE_2B_TILED_THICK,
	GX2_TILE_MODE_3D_TILED_THIN1,
	GX2_TILE_MODE_3D_TILED_THICK,
	GX2_TILE_MODE_3B_TILED_THIN1,
	GX2_TILE_MODE_3B_TILED_THICK,
	GX2_TILE_MODE_LINEAR_SPECIAL, // typically not supported by HW
	GX2_TILE_MODE_LINEAR_SPECIAL2
};

enum GX2SurfaceDim
{
	GX2_SURFACE_DIM_1D = 0,
	GX2_SURFACE_DIM_2D,
	GX2_SURFACE_DIM_3D,
	GX2_SURFACE_DIM_CUBE,
	GX2_SURFACE_DIM_1D_ARRAY,
	GX2_SURFACE_DIM_2D_ARRAY,
	GX2_SURFACE_DIM_2D_MSAA,
	GX2_SURFACE_DIM_2D_MSAA_ARRAY,
};


enum GX2AAMode
{
	GX2_AA_MODE_1X = 0,
	GX2_AA_MODE_2X,
	GX2_AA_MODE_4X,
	GX2_AA_MODE_8X,
};

enum GX2SurfaceFormat
{
	// color write performance relative to hardware peak write (x%)
	// texture read performance relative to hardware peak read (y%)
	// these numbers do not consider memory bandwidth limit
	// there are still some investigations for missing areas
	GX2_SURFACE_FORMAT_INVALID = 0x00000000,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_UNORM = 0x00000001,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_UINT = 0x00000101,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_SNORM = 0x00000201,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_SINT = 0x00000301,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_R4_G4_UNORM = 0x00000002,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TCD_R16_UNORM = 0x00000005,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R16_UINT = 0x00000105,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R16_SNORM = 0x00000205,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R16_SINT = 0x00000305,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R16_FLOAT = 0x00000806,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_G8_UNORM = 0x00000007,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_G8_UINT = 0x00000107,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_G8_SNORM = 0x00000207,
	// color write (50%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R8_G8_SINT = 0x00000307,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TCS_R5_G6_B5_UNORM = 0x00000008,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R5_G5_B5_A1_UNORM = 0x0000000a,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_R4_G4_B4_A4_UNORM = 0x0000000b,
	// color write (100%), texture read (100%)
	GX2_SURFACE_FORMAT_TC_A1_B5_G5_R5_UNORM = 0x0000000c,  ///< flipped
													   /// color write (50%)
	GX2_SURFACE_FORMAT_TC_R32_UINT = 0x0000010d,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R32_SINT = 0x0000030d,
	// color write (50%)
	GX2_SURFACE_FORMAT_TCD_R32_FLOAT = 0x0000080e,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_UNORM = 0x0000000f,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_UINT = 0x0000010f,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_SNORM = 0x0000020f,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_SINT = 0x0000030f,
	// color write (100%)
	GX2_SURFACE_FORMAT_TC_R16_G16_FLOAT = 0x00000810,
	GX2_SURFACE_FORMAT_D_D24_S8_UNORM = 0x00000011, ///< note: same value as below
	GX2_SURFACE_FORMAT_T_R24_UNORM_X8 = 0x00000011, ///< see Note 1
	GX2_SURFACE_FORMAT_T_X24_G8_UINT = 0x00000111, ///< see Note 1
	GX2_SURFACE_FORMAT_D_D24_S8_FLOAT = 0x00000811,
	// color write (100%)
	GX2_SURFACE_FORMAT_TC_R11_G11_B10_FLOAT = 0x00000816,
	// color write (100%)
	GX2_SURFACE_FORMAT_TCS_R10_G10_B10_A2_UNORM = 0x00000019,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R10_G10_B10_A2_UINT = 0x00000119,
	// color write (100%)
	GX2_SURFACE_FORMAT_TC_R10_G10_B10_A2_SNORM = 0x00000219, ///< A2 part is UNORM
															 /// color write (50%)
	GX2_SURFACE_FORMAT_TC_R10_G10_B10_A2_SINT = 0x00000319,
	// color write (100%)
	GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM = 0x0000001a,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R8_G8_B8_A8_UINT = 0x0000011a,
	// color write (100%)
	GX2_SURFACE_FORMAT_TC_R8_G8_B8_A8_SNORM = 0x0000021a,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R8_G8_B8_A8_SINT = 0x0000031a,
	// color write (100%)
	GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_SRGB = 0x0000041a,
	// color write (100%)
	GX2_SURFACE_FORMAT_TCS_A2_B10_G10_R10_UNORM = 0x0000001b, ///< flipped
															  /// color write (50%)
	GX2_SURFACE_FORMAT_TC_A2_B10_G10_R10_UINT = 0x0000011b, ///< flipped
	GX2_SURFACE_FORMAT_D_D32_FLOAT_S8_UINT_X24 = 0x0000081c, ///< note: same value as below
	GX2_SURFACE_FORMAT_T_R32_FLOAT_X8_X24 = 0x0000081c, ///< note: same value as above
	GX2_SURFACE_FORMAT_T_X32_G8_UINT_X24 = 0x0000011c, ///< see Note 2
													   /// color write (50%)
	GX2_SURFACE_FORMAT_TC_R32_G32_UINT = 0x0000011d,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R32_G32_SINT = 0x0000031d,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R32_G32_FLOAT = 0x0000081e,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_UNORM = 0x0000001f,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_UINT = 0x0000011f,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_SNORM = 0x0000021f,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_SINT = 0x0000031f,
	// color write (50%)
	GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_FLOAT = 0x00000820,
	// color write (25%)
	GX2_SURFACE_FORMAT_TC_R32_G32_B32_A32_UINT = 0x00000122,
	// color write (25%)
	GX2_SURFACE_FORMAT_TC_R32_G32_B32_A32_SINT = 0x00000322,
	// color write (25%)
	GX2_SURFACE_FORMAT_TC_R32_G32_B32_A32_FLOAT = 0x00000823,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC1_UNORM = 0x00000031,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC1_SRGB = 0x00000431,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC2_UNORM = 0x00000032,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC2_SRGB = 0x00000432,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC3_UNORM = 0x00000033,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC3_SRGB = 0x00000433,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC4_UNORM = 0x00000034,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC4_SNORM = 0x00000234,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC5_UNORM = 0x00000035,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_BC5_SNORM = 0x00000235,
	// texture read (100%)
	GX2_SURFACE_FORMAT_T_NV12_UNORM = 0x00000081, //< see Note 3
};

enum GX2SurfaceHWFormat
{
	GX2_SURFACE_HWFORMAT_INVALID = 0,
	GX2_SURFACE_HWFORMAT_R8 = 1,
	GX2_SURFACE_HWFORMAT_R4_G4 = 2,
	GX2_SURFACE_HWFORMAT_R16 = 5,
	GX2_SURFACE_HWFORMAT_R16_FLOAT = 6,
	GX2_SURFACE_HWFORMAT_R8_G8 = 7,
	GX2_SURFACE_HWFORMAT_R5_G6_B5 = 8,
	GX2_SURFACE_HWFORMAT_R5_G5_B5_A1 = 0xa,
	GX2_SURFACE_HWFORMAT_R4_G4_B4_A4 = 0xb,
	GX2_SURFACE_HWFORMAT_A1_B5_G5_R5 = 0xc,
	GX2_SURFACE_HWFORMAT_R32 = 0xd,
	GX2_SURFACE_HWFORMAT_R32_FLOAT = 0xe,
	GX2_SURFACE_HWFORMAT_R16_G16 = 0xf,
	GX2_SURFACE_HWFORMAT_R16_G16_FLOAT = 0x10,
	GX2_SURFACE_HWFORMAT_R24_S8 = 0x11,
	GX2_SURFACE_HWFORMAT_R11_G11_B10 = 0x16,
	GX2_SURFACE_HWFORMAT_R10_G10_B10_A2 = 0x19,
	GX2_SURFACE_HWFORMAT_R8_G8_B8_A8 = 0x1a,
	GX2_SURFACE_HWFORMAT_A2_B10_G10_R10 = 0x1b,
	GX2_SURFACE_HWFORMAT_D32_S8_X24 = 0x1c,
	GX2_SURFACE_HWFORMAT_R32_G32 = 0x1d,
	GX2_SURFACE_HWFORMAT_R32_G32_FLOAT = 0x1e,
	GX2_SURFACE_HWFORMAT_R16_G16_B16_A16 = 0x1f,
	GX2_SURFACE_HWFORMAT_R16_G16_B16_A16_FLOAT = 0x20,
	GX2_SURFACE_HWFORMAT_R32_G32_B32_A32 = 0x22,
	GX2_SURFACE_HWFORMAT_R32_G32_B32_A32_FLOAT = 0x23,
	GX2_SURFACE_HWFORMAT_BC1 = 0x31,
	GX2_SURFACE_HWFORMAT_BC2 = 0x32,
	GX2_SURFACE_HWFORMAT_BC3 = 0x33,
	GX2_SURFACE_HWFORMAT_BC4 = 0x34,
	GX2_SURFACE_HWFORMAT_BC5 = 0x35,
};

struct AddrLibMacroTilePrecomp
{
	unsigned int microTileThickness,
		microTileBits,
		microTileBytes,
		numSamples,
		swizzle_,
		sliceBytes,
		macroTileAspectRatio,
		macroTilePitch,
		macroTileHeight,
		macroTilesPerRow,
		macroTileBytes,
		bankSwapWidth,
		swappedBank,
		bpp;
	mutable unsigned int sampleSlice;

	AddrLibMacroTilePrecomp(unsigned int _bpp, unsigned int pitch, unsigned int height,
		GX2TileMode tileMode, unsigned int pipeSwizzle, unsigned int bankSwizzle);
};

struct AddrLibMicroTilePrecomp
{
	unsigned int microTileBytes,
		microTilesPerRow;
	unsigned int bpp;

	AddrLibMicroTilePrecomp(unsigned int _bpp, unsigned int pitch, GX2TileMode tileMode);
};

unsigned int computeSurfaceAddrFromCoordMacroTiled(unsigned int x, unsigned int y, const AddrLibMacroTilePrecomp &precomp);

unsigned int computeSurfaceAddrFromCoordMicroTiled(unsigned int x, unsigned int y, const AddrLibMicroTilePrecomp &precomp);