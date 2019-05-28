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

#include "png.h"
#include "datas/masterprinter.hpp"
#include <fstream>


void _pngerrorfunc(png_structp, png_const_charp error_msg)
{
	printerror("[PNG] ", << error_msg);
}

void _pngwarningfunc(png_structp, png_const_charp error_msg)
{
	printwarning("[PNG] ", << error_msg);
}

void _pngwritefunc(png_structp png_ptr, png_bytep data, png_size_t length)
{
	reinterpret_cast<std::ofstream*>(png_get_io_ptr(png_ptr))->write(reinterpret_cast<char*>(data), length);
}

void _pngflushfunc(png_structp png_ptr)
{
	reinterpret_cast<std::ofstream*>(png_get_io_ptr(png_ptr))->flush();
}

void WritePng(std::ofstream *stream, const char *buffer, int /*size*/, int width, int height, int colorType, int bpr, bool flipRB)
{
	png_structp pngStruct = nullptr;
	png_infop pngInfo = nullptr;
	png_voidp user_error_ptr = nullptr;

	pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		user_error_ptr, _pngerrorfunc, _pngwarningfunc);

	if (!pngStruct)
		return;

	if (setjmp(png_jmpbuf(pngStruct)))
		goto _pngexpEnd;

	pngInfo = png_create_info_struct(pngStruct);

	if (!pngInfo)
		goto _pngexpEnd;

	png_set_write_fn(pngStruct, stream, _pngwritefunc, _pngflushfunc);

	png_set_IHDR(pngStruct, pngInfo, width, height, 8, colorType,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(pngStruct, pngInfo);

	if (flipRB)
		png_set_bgr(pngStruct);

	for (int r = 0; r < height; r++)
		png_write_row(pngStruct, reinterpret_cast<const unsigned char*>(buffer + (r * width * bpr)));

	png_write_end(pngStruct, pngInfo);

_pngexpEnd:
	png_destroy_write_struct(&pngStruct, &pngInfo);
	return;
}