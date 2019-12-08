#pragma once
#include "datas/supercore.hpp"

template <class C, bool X64> union _MTHSPointer;

template <class C> union _MTHSPointer<C, true> {
  int offset;
  void Fixup(char *){};
  C *Get(char *masterBuffer) {
    return reinterpret_cast<C *>(masterBuffer + offset);
  }
};

template <class C> union _MTHSPointer<C, false> {
  int varPtr;
  C *ptr;
  char *cPtr;
  void Fixup(char *masterBuffer) {
    cPtr = masterBuffer + varPtr;
  };
  C *Get(char *) { return ptr; }
};

template <class C> using MTHSPointer = _MTHSPointer<C, ES_X64>;

class MTHSVertexShaderHeader;
class MTHSPixelShaderHeader;
class MTHSGeometryShaderHeader;

class MTHSHeader {
public:
  int magic, version;

private:
  MTHSPointer<MTHSVertexShaderHeader> vertexShader;
  MTHSPointer<MTHSPixelShaderHeader> pixelShader;
  MTHSPointer<MTHSGeometryShaderHeader> geometryShader;
  MTHSPointer<char> samplers, uniformVars, attributes, uniformBlocks, registers,
      varsNames, programs;

  char *GetMe() { return reinterpret_cast<char *>(this); }

public:
  MTHSVertexShaderHeader *GetVertexShader() {
    return vertexShader.Get(GetMe());
  }
  MTHSPixelShaderHeader *GetPixelShader() {
    return pixelShader.Get(GetMe());
  }
  MTHSGeometryShaderHeader *GetGeometryShader() {
    return geometryShader.Get(GetMe());
  }
  char *_GetSamplers() { return samplers.Get(GetMe()); }
  char *_GetUniformVars() { return uniformVars.Get(GetMe()); }
  char *_GetAttributes() { return attributes.Get(GetMe()); }
  char *_GetUniformBlocks() {
    return uniformBlocks.Get(GetMe());
  }
  char *_GetRegisters() { return registers.Get(GetMe()); }
  char *_GetNames() { return varsNames.Get(GetMe()); }
  char *_GetPrograms() { return programs.Get(GetMe()); }
  void SwapEndian();
};

class MTHSSampler {
  MTHSPointer<char> name;

public:
  int type, location;

  const char *Getname(MTHSHeader *hdr) {
    return name.Get(hdr->_GetNames());
  }

  void SwapEndian(MTHSHeader *hdr);
};

class MTHSUniformValue {
  MTHSPointer<char> name;

public:
  int varType, arrayCount, offset, blockIndex;

  const char *Getname(MTHSHeader *hdr) {
    return name.Get(hdr->_GetNames());
  }
  void SwapEndian(MTHSHeader *hdr);
};

class MTHSUniformBlock {
  MTHSPointer<char> name;

public:
  int offset, size;

  const char *Getname(MTHSHeader *hdr) {
    return name.Get(hdr->_GetNames());
  }
  void SwapEndian(MTHSHeader *hdr);
};

class MTHSAttribute {
  MTHSPointer<char> name;

public:
  int varType, arrayCount, location;

  const char *Getname(MTHSHeader *hdr) {
    return name.Get(hdr->_GetNames());
  }
  void SwapEndian(MTHSHeader *hdr);
};

class MTHSPixelShaderHeader {
  MTHSPointer<int> registry;
  int registryCount, programSize;
  MTHSPointer<char> program;
  int shaderMode, samplersCount;
  MTHSPointer<MTHSSampler> samplers;
  int uniformVarsCount;
  MTHSPointer<MTHSUniformValue> uniformVars;
  int null01[4], uniformBlocksCount;
  MTHSPointer<MTHSUniformBlock> uniformBlocks;

public:
  int *GetRegitry(MTHSHeader *hdr) {
    return registry.Get(hdr->_GetRegisters());
  }
  int NumRegisters() const { return registryCount; }
  char *GetProgram(MTHSHeader *hdr) {
    return program.Get(hdr->_GetPrograms());
  }
  int ProgramSize() const { return programSize; }
  MTHSSampler *GetSamplers(MTHSHeader *hdr) {
    return samplers.Get(hdr->_GetSamplers());
  }
  int NumSamplers() const { return samplersCount; }
  MTHSUniformBlock *GetUniformBlocks(MTHSHeader *hdr) {
    return uniformBlocks.Get(hdr->_GetUniformBlocks());
  }
  int NumUniformBlocks() const { return uniformBlocksCount; }
  MTHSUniformValue *GetUniformValues(MTHSHeader *hdr) {
    return uniformVars.Get(hdr->_GetUniformVars());
  }
  int NumUniformVars() const { return uniformVarsCount; }
  void SwapEndian(MTHSHeader *hdr);
};

class MTHSVertexShaderHeader : public MTHSPixelShaderHeader {
  int attributesCount;
  MTHSPointer<MTHSAttribute> attributes;
  int null02[6];

public:
  MTHSAttribute *GetAttributes(MTHSHeader *hdr) {
    return attributes.Get(hdr->_GetAttributes());
  }
  int NumAttributes() const { return attributesCount; }
  void SwapEndian(MTHSHeader *hdr);
};

class MTHS {
  static constexpr int ID = CompileFourCC("MTHS");
  static constexpr int IDs = CompileFourCC("SHTM");
  union {
    void *linked;
    char *masterBuffer;
    MTHSHeader *header;
  } data;
  bool linked;
  template <class _Ty0>
  // typedef wchar_t _Ty0;
  int _Load(const _Ty0 *fileName, bool suppressErrors);

public:
  MTHS() : data(), linked(false) {}
  ~MTHS();

  int Load(const char *fileName, bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }
  int Load(const wchar_t *fileName,
                          bool suppressErrors = false) {
    return _Load(fileName, suppressErrors);
  }
  const MTHSHeader *GetShader() const { return data.header; }
  int Link(void *file);
};