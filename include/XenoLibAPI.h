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

#pragma once

struct TextureConversionParams
{
	bool uncompress : 1, allowBC5ZChan : 1, reserved : 6;
};

int ConvertMTXT(const char *buffer, int size, const char *path, TextureConversionParams params);
int ConvertMTXT(const char *buffer, int size, const wchar_t *path, TextureConversionParams params);
int ConvertLBIM(const char *buffer, int size, const char *path, TextureConversionParams params, const char *exBuffer = 0, int exBuffSize = 0);
int ConvertLBIM(const char *buffer, int size, const wchar_t *path, TextureConversionParams params, const char *exBuffer = 0, int exBuffSize = 0);

char *ExtractXBC(const char *buffer);