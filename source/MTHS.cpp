#include "MTHS.h"
#include "datas/endian.hpp"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"

template<class E, class C> ES_INLINE void _ArraySwap(C &input)
{
	const size_t numItems = sizeof(C) / sizeof(E);
	E *inputPtr = reinterpret_cast<E *>(&input);

	for (size_t t = 0; t < numItems; t++)
		FByteswapper(*(inputPtr + t));
}

template<class _Ty0>
int MTHS::_Load(const _Ty0 *fileName, bool suppressErrors)
{
	BinReader rd(fileName);

	if (!rd.IsValid())
	{
		if (!suppressErrors)
		{
			printerror("[MTHS] Cannot load file: ", << fileName);
		}

		return 1;
	}

	int magic;
	rd.Read(magic);

	if (magic != ID)
	{
		if (!suppressErrors)
		{
			printerror("[MTHS] Invalid header.");
		}

		return 2;
	}

	rd.Seek(0);

	const size_t fileSize = rd.GetSize();

	data.masterBuffer = static_cast<char *>(malloc(fileSize));
	rd.ReadBuffer(data.masterBuffer, fileSize);
	data.header->SwapEndian();

	return 0;
}

int MTHS::Link(void *file)
{
	data.linked = file;

	if (data.header->magic == ID)
	{
		data.header->SwapEndian();
	}
	else if (data.header->magic != IDs)
	{
		printerror("[MTHS] Invalid header.");
		return 1;
	}

	linked = true;
	return 0;
}

MTHS::~MTHS()
{
	if (data.masterBuffer && !linked)
		free(data.masterBuffer);
}

ES_INLINE void MTHSHeader::SwapEndian()
{
	_ArraySwap<int>(*this);

	vertexShader.Fixup(GetMe());
	pixelShader.Fixup(GetMe());
	geometryShader.Fixup(GetMe());
	samplers.Fixup(GetMe());
	uniformVars.Fixup(GetMe());
	attributes.Fixup(GetMe());
	uniformBlocks.Fixup(GetMe());
	registers.Fixup(GetMe());
	varsNames.Fixup(GetMe());
	programs.Fixup(GetMe());

	vertexShader.Get(GetMe())->SwapEndian(this);
	pixelShader.Get(GetMe())->SwapEndian(this);
}

ES_INLINE void MTHSPixelShaderHeader::SwapEndian(MTHSHeader *hdr)
{
	_ArraySwap<int>(*this);

	registry.Fixup(hdr->_GetRegisters());
	program.Fixup(hdr->_GetPrograms());
	samplers.Fixup(hdr->_GetSamplers());
	uniformVars.Fixup(hdr->_GetUniformVars());
	uniformBlocks.Fixup(hdr->_GetUniformBlocks());
	
	int *cReg = GetRegitry(hdr);

	for (int r = 0; r < NumRegisters(); r++)
		FByteswapper(cReg[r]);

	MTHSSampler *sampl = GetSamplers(hdr);

	for (int s = 0; s < NumSamplers(); s++)
		sampl[s].SwapEndian(hdr);

	MTHSUniformBlock *blocks = GetUniformBlocks(hdr);

	for (int b = 0; b < NumUniformBlocks(); b++)
		blocks[b].SwapEndian(hdr);

	MTHSUniformValue *vars = GetUniformValues(hdr);

	for (int v = 0; v < NumUniformVars(); v++)
		vars[v].SwapEndian(hdr);
}

ES_INLINE void MTHSVertexShaderHeader::SwapEndian(MTHSHeader *hdr)
{
	MTHSPixelShaderHeader::SwapEndian(hdr);

	FByteswapper(attributes);
	FByteswapper(attributesCount);

	attributes.Fixup(hdr->_GetAttributes());

	MTHSAttribute *attr = GetAttributes(hdr);

	for (int a = 0; a < NumAttributes(); a++)
		attr[a].SwapEndian(hdr);
}

ES_INLINE void MTHSSampler::SwapEndian(MTHSHeader *hdr)
{
	_ArraySwap<int>(*this);

	name.Fixup(hdr->_GetNames());
}

ES_INLINE void MTHSUniformValue::SwapEndian(MTHSHeader *hdr)
{
	_ArraySwap<int>(*this);

	name.Fixup(hdr->_GetNames());
}

ES_INLINE void MTHSUniformBlock::SwapEndian(MTHSHeader *hdr)
{
	_ArraySwap<int>(*this);

	name.Fixup(hdr->_GetNames());
}

ES_INLINE void MTHSAttribute::SwapEndian(MTHSHeader *hdr)
{
	_ArraySwap<int>(*this);

	name.Fixup(hdr->_GetNames());
}