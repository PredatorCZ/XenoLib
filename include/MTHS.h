#pragma once
#include "datas/supercore.hpp"

template<class C, bool X64> union _MTHSPointer;

template<class C> union _MTHSPointer<C, true>
{
	int offset;
	ES_FORCEINLINE void Fixup(char *) {};
	ES_FORCEINLINE C *Get(char *masterBuffer) { return reinterpret_cast<C *>(masterBuffer + offset); }
};

template<class C> union _MTHSPointer<C, false>
{
	int varPtr;
	C *ptr;
	char *cPtr;
	ES_FORCEINLINE void Fixup(char *masterBuffer) { cPtr = masterBuffer + varPtr; };
	ES_FORCEINLINE C *Get(char *) { return ptr; }
};

template <class C> using MTHSPointer = _MTHSPointer<C, ES_X64>;

class MTHSVertexShaderHeader;
class MTHSPixelShaderHeader;
class MTHSGeometryShaderHeader;

class MTHSHeader
{
public:
	int magic,
		version;
private:
	MTHSPointer<MTHSVertexShaderHeader> vertexShader;
	MTHSPointer<MTHSPixelShaderHeader> pixelShader;
	MTHSPointer<MTHSGeometryShaderHeader> geometryShader;
	MTHSPointer<char> samplers,
		uniformVars,
		attributes,
		uniformBlocks,
		registers,
		varsNames,
		programs;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }
public:
	ES_FORCEINLINE MTHSVertexShaderHeader *GetVertexShader() { return vertexShader.Get(GetMe()); }
	ES_FORCEINLINE MTHSPixelShaderHeader *GetPixelShader() { return pixelShader.Get(GetMe()); }
	ES_FORCEINLINE MTHSGeometryShaderHeader *GetGeometryShader() { return geometryShader.Get(GetMe()); }
	ES_FORCEINLINE char *_GetSamplers() { return samplers.Get(GetMe()); }
	ES_FORCEINLINE char *_GetUniformVars() { return uniformVars.Get(GetMe()); }
	ES_FORCEINLINE char *_GetAttributes() { return attributes.Get(GetMe()); }
	ES_FORCEINLINE char *_GetUniformBlocks() { return uniformBlocks.Get(GetMe()); }
	ES_FORCEINLINE char *_GetRegisters() { return registers.Get(GetMe()); }
	ES_FORCEINLINE char *_GetNames() { return varsNames.Get(GetMe()); }
	ES_FORCEINLINE char *_GetPrograms() { return programs.Get(GetMe()); }

	void SwapEndian();
};

class MTHSSampler
{
	MTHSPointer<char> name;
public:
	int type,
		location;

	ES_FORCEINLINE const char *Getname(MTHSHeader *hdr) { return name.Get(hdr->_GetNames()); }

	void SwapEndian(MTHSHeader *hdr);
};

class MTHSUniformValue
{
	MTHSPointer<char> name;
public:
	int varType,
		arrayCount,
		offset,
		blockIndex;

	ES_FORCEINLINE const char *Getname(MTHSHeader *hdr) { return name.Get(hdr->_GetNames()); }

	void SwapEndian(MTHSHeader *hdr);
};

class MTHSUniformBlock
{
	MTHSPointer<char> name;
public:
	int offset,
		size;

	ES_FORCEINLINE const char *Getname(MTHSHeader *hdr) { return name.Get(hdr->_GetNames()); }

	void SwapEndian(MTHSHeader *hdr);
};

class MTHSAttribute
{
	MTHSPointer<char> name;
public:
	int varType,
		arrayCount,
		location;

	ES_FORCEINLINE const char *Getname(MTHSHeader *hdr) { return name.Get(hdr->_GetNames()); }

	void SwapEndian(MTHSHeader *hdr);
};

class MTHSPixelShaderHeader
{
	MTHSPointer<int> registry;
	int registryCount,
		programSize;
	MTHSPointer<char> program;
	int shaderMode,
		samplersCount;
	MTHSPointer<MTHSSampler> samplers;
	int uniformVarsCount;
	MTHSPointer<MTHSUniformValue> uniformVars;
	int null01[4],
		uniformBlocksCount;
	MTHSPointer<MTHSUniformBlock> uniformBlocks;
public:
	ES_FORCEINLINE int *GetRegitry(MTHSHeader *hdr) { return registry.Get(hdr->_GetRegisters()); }
	ES_FORCEINLINE int NumRegisters() const { return registryCount; }
	ES_FORCEINLINE char *GetProgram(MTHSHeader *hdr) { return program.Get(hdr->_GetPrograms()); }
	ES_FORCEINLINE int ProgramSize() const { return programSize; }
	ES_FORCEINLINE MTHSSampler *GetSamplers(MTHSHeader *hdr) { return samplers.Get(hdr->_GetSamplers()); }
	ES_FORCEINLINE int NumSamplers() const { return samplersCount; }
	ES_FORCEINLINE MTHSUniformBlock *GetUniformBlocks(MTHSHeader *hdr) { return uniformBlocks.Get(hdr->_GetUniformBlocks()); }
	ES_FORCEINLINE int NumUniformBlocks() const { return uniformBlocksCount; }
	ES_FORCEINLINE MTHSUniformValue *GetUniformValues(MTHSHeader *hdr) { return uniformVars.Get(hdr->_GetUniformVars()); }
	ES_FORCEINLINE int NumUniformVars() const { return uniformVarsCount; }

	void SwapEndian(MTHSHeader *hdr);
};

class MTHSVertexShaderHeader : public MTHSPixelShaderHeader
{
	int attributesCount;
	MTHSPointer<MTHSAttribute> attributes;
	int null02[6];
public:
	ES_FORCEINLINE MTHSAttribute *GetAttributes(MTHSHeader *hdr) { return attributes.Get(hdr->_GetAttributes()); }
	ES_FORCEINLINE int NumAttributes() const { return attributesCount; }

	void SwapEndian(MTHSHeader *hdr);
};


class MTHS
{
	static const int ID = CompileFourCC("MTHS");
	static const int IDs = CompileFourCC("SHTM");
	union
	{
		void *linked;
		char *masterBuffer;
		MTHSHeader *header;
	}data;
	bool linked;
	template<class _Ty0>
	//typedef wchar_t _Ty0;
	int _Load(const _Ty0 *fileName, bool suppressErrors);
public:
	MTHS() : data(), linked(false) {}
	~MTHS();

	ES_FORCEINLINE int Load(const char *fileName, bool suppressErrors = false) { return _Load(fileName, suppressErrors); }
	ES_FORCEINLINE int Load(const wchar_t *fileName, bool suppressErrors = false) { return _Load(fileName, suppressErrors); }
	ES_FORCEINLINE const MTHSHeader *GetShader() const { return data.header; }

	int Link(void *file);
};