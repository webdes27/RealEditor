#include "Core.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <array>
#include <bitset>
#include <cstring>
#define NOGDICAPMASKS
#define NOMENUS
#define NOATOM
#define NODRAWTEXT
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NOSHOWWINDOW
#define NOWINSTYLES
#define NOSYSMETRICS
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT
#define NOTAPE
#define NOIMAGE
#define NOPROXYSTUB
#define NORPC
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ppl.h>
#include <minilzo/minilzo.h>

#include "ALog.h"

#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

#define COMPRESSED_BLOCK_MAGIC PACKAGE_MAGIC
#define COMPRESSION_FLAGS_TYPE_MASK		0x0F
#define COMPRESSION_FLAGS_OPTIONS_MASK	0xF0

bool _HasAVX2()
{
  std::array<int, 4> cpui;
  std::vector<std::array<int, 4>> data;
  std::bitset<32> f_7_EBX_;
  __cpuid(cpui.data(), 0);
  int nIds = cpui[0];
  for (int i = 0; i <= nIds; ++i)
  {
    __cpuidex(cpui.data(), i, 0);
    data.push_back(cpui);
  }
  if (nIds >= 7)
  {
    f_7_EBX_ = data[7][1];
  }
  return f_7_EBX_[5];
}

std::string W2A(const wchar_t* str, int32 len)
{
  if (len == -1)
  {
    len = lstrlenW(str);
  }
  int32 size = WideCharToMultiByte(CP_UTF8, 0, str, len, NULL, 0, NULL, NULL);
  std::string result(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, str, len, &result[0], size, NULL, NULL);
  return result;
}

std::string W2A(const std::wstring& str)
{
  return W2A(&str[0], (int32)str.length());
}

std::wstring A2W(const char* str, int32 len)
{
  if (len == -1)
  {
    len = (int32)strlen(str);
  }
  int32 size = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
  std::wstring result(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, str, len, &result[0], size);
  return result;
}

std::wstring A2W(const std::string& str)
{
  return A2W(&str[0], (int32)str.length());
}

uint64 GetFileTime(const std::wstring& path)
{
  struct _stat64 fileInfo;
  if (!_wstati64(path.c_str(), &fileInfo))
  {
    return fileInfo.st_mtime;
  }
  return 0;
}

#include <wx/string.h>

void UThrow(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  std::string msg = Sprintf(fmt, ap);
  va_end(ap);
  LogE(msg);
  throw std::runtime_error(msg);
}

void UThrow(const wchar* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  std::string msg = Sprintf(fmt, ap);
  va_end(ap);
  LogE(msg);
  throw std::runtime_error(msg);
}

bool IsAnsi(const std::string& str)
{
  std::wstring wstr = A2W(str);
  for (const wchar& ch : wstr)
  {
    if (ch > 127)
    {
      return false;
    }
  }
  return true;
}

bool IsAnsi(const std::wstring& str)
{
  for (const wchar& ch : str)
  {
    if (ch > 127)
    {
      return false;
    }
  }
  return true;
}

std::string Sprintf(const char* fmt, ...)
{
  int final_n, n = ((int)strlen(fmt)) * 2;
  std::unique_ptr<char[]> formatted;
  va_list ap;
  while (1)
  {
    formatted.reset(new char[n]);
    strcpy(&formatted[0], fmt);
    va_start(ap, fmt);
    final_n = vsnprintf(&formatted[0], n, fmt, ap);
    va_end(ap);
    if (final_n < 0 || final_n >= n)
    {
      n += abs(final_n - n + 1);
    }
    else
    {
      break;
    }
  }
  return std::string(formatted.get());
}

std::string Sprintf(const char* fmt, va_list ap)
{
  int final_n, n = ((int)strlen(fmt)) * 2;
  std::unique_ptr<char[]> formatted;
  va_list tmp;
  while (1)
  {
    formatted.reset(new char[n]);
    strcpy(&formatted[0], fmt);
    va_copy(tmp, ap);
    final_n = vsnprintf(&formatted[0], n, fmt, tmp);
    va_end(tmp);
    if (final_n < 0 || final_n >= n)
    {
      n += abs(final_n - n + 1);
    }
    else
    {
      break;
    }
  }
  return std::string(formatted.get());
}

std::string Sprintf(const wchar* fmt, va_list ap)
{
  int final_n, n = ((int)wcslen(fmt)) * 2;
  std::unique_ptr<wchar[]> formatted;
  va_list tmp;
  while (1)
  {
    formatted.reset(new wchar[n]);
    wcscpy(&formatted[0], fmt);
    va_copy(tmp, ap);
    final_n = _vsnwprintf(&formatted[0], n, fmt, tmp);
    va_end(tmp);
    if (final_n < 0 || final_n >= n)
    {
      n += abs(final_n - n + 1);
    }
    else
    {
      break;
    }
  }
  return W2A(formatted.get());
}

std::wstring Sprintf(const wchar* fmt, ...)
{
  int final_n, n = ((int)wcslen(fmt)) * 2;
  std::unique_ptr<wchar[]> formatted;
  va_list ap;
  while (1)
  {
    formatted.reset(new wchar[n]);
    wcscpy(&formatted[0], fmt);
    va_start(ap, fmt);
    final_n = _vsnwprintf(&formatted[0], n, fmt, ap);
    va_end(ap);
    if (final_n < 0 || final_n >= n)
    {
      n += abs(final_n - n + 1);
    }
    else
    {
      break;
    }
  }
  return std::wstring(formatted.get());
}

std::string Sprintf(const std::string fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  std::string msg = Sprintf(fmt.c_str(), ap);
  va_end(ap);
  return msg;
}

void memswap(void* a, void* b, size_t size)
{
  if (!size)
  {
    return;
  }
  void* tmp = malloc(size);
  memcpy(tmp, a, size);
  memcpy(a, b, size);
  memcpy(b, tmp, size);
  free(tmp);
}

void LZO::Decompress(const void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET dstSize, bool concurrent)
{
  lzo_bytep ptr = (lzo_bytep)src;
  lzo_bytep start = ptr;
  lzo_bytep dstStart = (lzo_bytep)dst;
  if (*(int*)ptr != COMPRESSED_BLOCK_MAGIC)
  {
    UThrow("Invalid or corrupted compression block!");
  }
  ptr += 4;
  uint32 blockSize = *(uint32*)ptr; ptr += 4;
  uint32 totalCompressedSize = *(uint32*)ptr; ptr += 4;
  uint32 totalDecompressedSize = *(uint32*)ptr; ptr += 4;
  int32 totalBlocks = (int32)ceilf((float)totalDecompressedSize / (float)blockSize);

  static bool err = (LZO_E_OK != lzo_init());

  if (err)
  {
    UThrow("Failed to initialize LZO!");
  }

  struct CompressionBlock {
    uint32 srcOffset = 0;
    uint32 srcSize = 0;
    uint32 dstOffset = 0;
    uint32 dstSize = 0;
  };

  const uint32 chunkHeaderSize = 16;
  const uint32 blockHeaderSize = 8;
  uint32 compressedOffset = 0;
  uint32 decompressedOffset = 0;

  CompressionBlock* compressionInfo = new CompressionBlock[totalBlocks];
  for (int i = 0; i < totalBlocks; i++)
  {
    compressionInfo[i].srcOffset = chunkHeaderSize + compressedOffset + totalBlocks * blockHeaderSize;
    compressionInfo[i].srcSize = *(uint32*)ptr; ptr += 4;
    compressionInfo[i].dstSize = *(uint32*)ptr; ptr += 4;
    compressionInfo[i].dstOffset = decompressedOffset;
    compressedOffset += compressionInfo[i].srcSize;
    decompressedOffset += compressionInfo[i].dstSize;
  }

  if (concurrent)
  {
    concurrency::parallel_for(int32(0), totalBlocks, [&](int32 i) {
      lzo_uint decompressedSize = compressionInfo[i].dstSize;
      int e = lzo1x_decompress_safe(start + compressionInfo[i].srcOffset, compressionInfo[i].srcSize, dstStart + compressionInfo[i].dstOffset, &decompressedSize, NULL);
      if (e != LZO_E_OK)
      {
        UThrow("Corrupted compression block. Code: %d", e);
      }
    });
  }
  else
  {
    for (int32 i = 0; i < totalBlocks; ++i)
    {
      lzo_uint decompressedSize = compressionInfo[i].dstSize;
      int e = lzo1x_decompress_safe(start + compressionInfo[i].srcOffset, compressionInfo[i].srcSize, dstStart + compressionInfo[i].dstOffset, &decompressedSize, NULL);
      if (e != LZO_E_OK)
      {
        UThrow("Corrupted compression block. Code: %d", e);
      }
    }
  }

  delete[] compressionInfo;
}

bool DecompressLZO(const void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET dstSize, bool concurrent)
{
  lzo_uint finalSize = dstSize;
  int e = lzo1x_decompress_safe((lzo_bytep)src, srcSize, (lzo_bytep)dst, &finalSize, NULL);
  if (e != LZO_E_OK)
  {
    LogE("Corrupted compression block. Code: %d", e);
    return false;
  }
  return true;
}

bool CompressLZO(const void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET* dstSize, bool concurrent)
{
  lzo_uint resultSize = *dstSize;
  int e = lzo1x_1_compress((lzo_bytep)src, srcSize, (unsigned char*)dst, &resultSize, wrkmem);
  if (e != LZO_E_OK)
  {
    LogE("Failed to compress memory. Code: %d", e);
    return false;
  }
  *dstSize = resultSize;
  return true;
}

bool DecompressMemory(ECompressionFlags flags, void* decompressedBuffer, int32 decompressedSize, const void* compressedBuffer, int32 compressedSize)
{
  bool ok = false;
  switch (flags & COMPRESSION_FLAGS_TYPE_MASK)
  {
  case COMPRESS_ZLIB:
    // TODO: implement zlib
    LogE("Zlib decompression not implemented!");
    DBreak();
    break;
  case COMPRESS_LZO:
    ok = DecompressLZO(compressedBuffer, compressedSize, decompressedBuffer, decompressedSize, true);
    break;
  case COMPRESS_LZX:
    // TODO: implement lzx
    LogE("Lzx decompression not implemented!");
    DBreak();
    break;
  default:
    LogE("Unknown compression format: %d", flags & COMPRESSION_FLAGS_TYPE_MASK);
    ok = false;
  }
  return ok;
}

bool CompressMemory(ECompressionFlags flags, void* compressedBuffer, int32* compressedSize, const void* decompressedBuffer, int32 decompressedSize)
{
  bool ok = false;
  switch (flags & COMPRESSION_FLAGS_TYPE_MASK)
  {
  case COMPRESS_ZLIB:
    // TODO: implement zlib
    LogE("Zlib compression not implemented!");
    DBreak();
    break;
  case COMPRESS_LZO:
    ok = CompressLZO(decompressedBuffer, decompressedSize, compressedBuffer, compressedSize, true);
    break;
  case COMPRESS_LZX:
    // TODO: implement lzx
    LogE("Lzx compression not implemented!");
    DBreak();
    break;
  default:
    LogE("Unknown compression format: %d", flags & COMPRESSION_FLAGS_TYPE_MASK);
    ok = false;
  }
  return ok;
}

FString ObjectFlagsToString(uint64 expFlag)
{
  FString s;
  if (expFlag & RF_InSingularFunc)
    s += "InSingularFunc, ";
  if (expFlag & RF_StateChanged)
    s += "StateChanged, ";
  if (expFlag & RF_DebugPostLoad)
    s += "DebugPostLoad, ";
  if (expFlag & RF_DebugSerialize)
    s += "DebugSerialize, ";
  if (expFlag & RF_DebugFinishDestroyed)
    s += "DebugFinishDestroyed, ";
  if (expFlag & RF_EdSelected)
    s += "EdSelected, ";
  if (expFlag & RF_ZombieComponent)
    s += "ZombieComponent, ";
  if (expFlag & RF_Protected)
    s += "Protected, ";
  if (expFlag & RF_ClassDefaultObject)
    s += "ClassDefaultObject, ";
  if (expFlag & RF_ArchetypeObject)
    s += "ArchetypeObject, ";
  if (expFlag & RF_ForceTagExp)
    s += "ForceTagExp, ";
  if (expFlag & RF_TokenStreamAssembled)
    s += "TokenStreamAssembled, ";
  if (expFlag & RF_MisalignedObject)
    s += "MisalignedObject, ";
  if (expFlag & RF_RootSet)
    s += "RootSet, ";
  if (expFlag & RF_BeginDestroyed)
    s += "BeginDestroyed, ";
  if (expFlag & RF_FinishDestroyed)
    s += "FinishDestroyed, ";
  if (expFlag & RF_DebugBeginDestroyed)
    s += "DebugBeginDestroyed, ";
  if (expFlag & RF_MarkedByCooker)
    s += "MarkedByCooker, ";
  if (expFlag & RF_LocalizedResource)
    s += "LocalizedResource, ";
  if (expFlag & RF_InitializedProps)
    s += "InitializedProps, ";
  if (expFlag & RF_PendingFieldPatches)
    s += "PendingFieldPatches, ";
  if (expFlag & RF_IsCrossLevelReferenced)
    s += "IsCrossLevelReferenced, ";
  if (expFlag & RF_DebugBeginDestroyed)
    s += "DebugBeginDestroyed, ";
  if (expFlag & RF_Saved)
    s += "Saved, ";
  if (expFlag & RF_Transactional)
    s += "Transactional, ";
  if (expFlag & RF_Unreachable)
    s += "Unreachable, ";
  if (expFlag & RF_Public)
    s += "Public, ";
  if (expFlag & RF_TagImp)
    s += "TagImp, ";
  if (expFlag & RF_TagExp)
    s += "TagExp, ";
  if (expFlag & RF_Obsolete)
    s += "Obsolete, ";
  if (expFlag & RF_TagGarbage)
    s += "TagGarbage, ";
  if (expFlag & RF_DisregardForGC)
    s += "DisregardForGC, ";
  if (expFlag & RF_PerObjectLocalized)
    s += "PerObjectLocalized, ";
  if (expFlag & RF_NeedLoad)
    s += "NeedLoad, ";
  if (expFlag & RF_AsyncLoading)
    s += "AsyncLoading, ";
  if (expFlag & RF_NeedPostLoadSubobjects)
    s += "NeedPostLoadSubobjects, ";
  if (expFlag & RF_Suppress)
    s += "Suppress, ";
  if (expFlag & RF_InEndState)
    s += "InEndState, ";
  if (expFlag & RF_Transient)
    s += "Transient, ";
  if (expFlag & RF_Cooked)
    s += "Cooked, ";
  if (expFlag & RF_LoadForClient)
    s += "LoadForClient, ";
  if (expFlag & RF_LoadForServer)
    s += "LoadForServer, ";
  if (expFlag & RF_LoadForEdit)
    s += "LoadForEdit, ";
  if (expFlag & RF_Standalone)
    s += "Standalone, ";
  if (expFlag & RF_NotForClient)
    s += "NotForClient, ";
  if (expFlag & RF_NotForServer)
    s += "NotForServer, ";
  if (expFlag & RF_NotForEdit)
    s += "NotForEdit, ";
  if (expFlag & RF_NeedPostLoad)
    s += "NeedPostLoad, ";
  if (expFlag & RF_HasStack)
    s += "HasStack, ";
  if (expFlag & RF_Native)
    s += "Native, ";
  if (expFlag & RF_Marked)
    s += "Marked, ";
  if (expFlag & RF_ErrorShutdown)
    s += "ErrorShutdown, ";
  if (expFlag & RF_NotForEdit)
    s += "NotForEdit, ";
  if (expFlag & RF_PendingKill)
    s += "PendingKill, ";
  if (s.Size())
  {
    s = s.Substr(0, s.Size() - 2);
  }
  else
  {
    s = "None";
  }
  return s;
}

FString ExportFlagsToString(uint32 flags)
{
  FString s;
  if (flags & EF_ForcedExport)
  {
    s += "ForcedExport, ";
  }
  if (flags & EF_ScriptPatcherExport)
  {
    s += "ScriptPatcherExport, ";
  }
  if (flags & EF_MemberFieldPatchPending)
  {
    s += "MemberFieldPatchPending, ";
  }
  if (s.Size())
  {
    s = s.Substr(0, s.Size() - 2);
  }
  else
  {
    s = "None";
  }
  return s;
}

FString PixelFormatToString(uint32 pf)
{
  if (pf == PF_DXT1)
  {
    return "PF_DXT1";
  }
  else if (pf == PF_DXT5)
  {
    return "PF_DXT5";
  }
  else if (pf == PF_A8R8G8B8)
  {
    return "PF_A8R8G8B8";
  }
  else if (pf == PF_A32B32G32R32F)
  {
    return "PF_A32B32G32R32F";
  }
  else if (pf == PF_G8)
  {
    return "PF_G8";
  }
  else if (pf == PF_G16)
  {
    return "PF_G16";
  }
  else if (pf == PF_DXT3)
  {
    return "PF_DXT3";
  }
  else if (pf == PF_UYVY)
  {
    return "PF_UYVY";
  }
  else if (pf == PF_FloatRGB)
  {
    return "PF_FloatRGB";
  }
  else if (pf == PF_FloatRGBA)
  {
    return "PF_FloatRGBA";
  }
  else if (pf == PF_DepthStencil)
  {
    return "PF_DepthStencil";
  }
  else if (pf == PF_ShadowDepth)
  {
    return "PF_ShadowDepth";
  }
  else if (pf == PF_FilteredShadowDepth)
  {
    return "PF_FilteredShadowDepth";
  }
  else if (pf == PF_R32F)
  {
    return "PF_R32F";
  }
  else if (pf == PF_G16R16)
  {
    return "PF_G16R16";
  }
  else if (pf == PF_G16R16F)
  {
    return "PF_G16R16F";
  }
  else if (pf == PF_G16R16F_FILTER)
  {
    return "PF_G16R16F_FILTER";
  }
  else if (pf == PF_G32R32F)
  {
    return "PF_G32R32F";
  }
  else if (pf == PF_A2B10G10R10)
  {
    return "PF_A2B10G10R10";
  }
  else if (pf == PF_A16B16G16R16)
  {
    return "PF_A16B16G16R16";
  }
  else if (pf == PF_D24)
  {
    return "PF_D24";
  }
  else if (pf == PF_R16F)
  {
    return "PF_R16F";
  }
  else if (pf == PF_R16F_FILTER)
  {
    return "PF_R16F_FILTER";
  }
  else if (pf == PF_BC5)
  {
    return "PF_BC5";
  }
  else if (pf == PF_V8U8)
  {
    return "PF_V8U8";
  }
  else if (pf == PF_A1)
  {
    return "PF_A1";
  }
  else if (pf == PF_FloatR11G11B10)
  {
    return "PF_FloatR11G11B10";
  }
  return "PF_Unknown";
}

FString PackageFlagsToString(uint32 flags)
{
  FString s;
  if (flags & PKG_AllowDownload)
    s += "AllowDownload, ";
  if (flags & PKG_ClientOptional)
    s += "ClientOptional, ";
  if (flags & PKG_ServerSideOnly)
    s += "ServerSideOnly, ";
  if (flags & PKG_Cooked)
    s += "Cooked, ";
  if (flags & PKG_Unsecure)
    s += "Unsecure, ";
  if (flags & PKG_SavedWithNewerVersion)
    s += "SavedWithNewerVersion, ";
  if (flags & PKG_Need)
    s += "Need, ";
  if (flags & PKG_Compiling)
    s += "Compiling, ";
  if (flags & PKG_ContainsMap)
    s += "ContainsMap, ";
  if (flags & PKG_Trash)
    s += "Trash, ";
  if (flags & PKG_DisallowLazyLoading)
    s += "DisallowLazyLoading, ";
  if (flags & PKG_PlayInEditor)
    s += "PlayInEditor, ";
  if (flags & PKG_ContainsScript)
    s += "ContainsScript, ";
  if (flags & PKG_ContainsDebugInfo)
    s += "ContainsDebugInfo, ";
  if (flags & PKG_RequireImportsAlreadyLoaded)
    s += "RequireImportsAlreadyLoaded, ";
  if (flags & PKG_SelfContainedLighting)
    s += "SelfContainedLighting, ";
  if (flags & PKG_StoreCompressed)
    s += "StoreCompressed, ";
  if (flags & PKG_StoreFullyCompressed)
    s += "StoreFullyCompressed, ";
  if (flags & PKG_ContainsInlinedShaders)
    s += "ContainsInlinedShaders, ";
  if (flags & PKG_ContainsFaceFXData)
    s += "ContainsFaceFXData, ";
  if (flags & PKG_NoExportAllowed)
    s += "NoExportAllowed, ";
  if (flags & PKG_NoExportAllowed)
    s += "StrippedSource, ";
  if (s.Size())
  {
    s = s.Substr(0, s.Size() - 2);
  }
  else
  {
    s = "None";
  }
  return s;
}

FString ClassFlagsToString(uint32 flags)
{
#define PARSE_CLASS_FLAGS(name) if (flags & CLASS_##name) s += #name + FString(", ")
  FString s;
  PARSE_CLASS_FLAGS(Abstract);
  PARSE_CLASS_FLAGS(Compiled);
  PARSE_CLASS_FLAGS(Config);
  PARSE_CLASS_FLAGS(Transient);
  PARSE_CLASS_FLAGS(Parsed);
  PARSE_CLASS_FLAGS(Localized);
  PARSE_CLASS_FLAGS(SafeReplace);
  PARSE_CLASS_FLAGS(Localized);
  PARSE_CLASS_FLAGS(SafeReplace);
  PARSE_CLASS_FLAGS(Native);
  PARSE_CLASS_FLAGS(NoExport);
  PARSE_CLASS_FLAGS(Placeable);
  PARSE_CLASS_FLAGS(PerObjectConfig);
  PARSE_CLASS_FLAGS(NativeReplication);
  PARSE_CLASS_FLAGS(EditInlineNew);
  PARSE_CLASS_FLAGS(CollapseCategories);
  PARSE_CLASS_FLAGS(Interface);
  PARSE_CLASS_FLAGS(HasInstancedProps);
  PARSE_CLASS_FLAGS(NeedsDefProps);
  PARSE_CLASS_FLAGS(HasComponents);
  PARSE_CLASS_FLAGS(Hidden);
  PARSE_CLASS_FLAGS(Deprecated);
  PARSE_CLASS_FLAGS(HideDropDown);
  PARSE_CLASS_FLAGS(Exported);
  PARSE_CLASS_FLAGS(Intrinsic);
  PARSE_CLASS_FLAGS(NativeOnly);
  PARSE_CLASS_FLAGS(PerObjectLocalized);
  PARSE_CLASS_FLAGS(HasCrossLevelRefs);
  PARSE_CLASS_FLAGS(IsAUProperty);
  PARSE_CLASS_FLAGS(IsAUObjectProperty);
  PARSE_CLASS_FLAGS(IsAUBoolProperty);
  PARSE_CLASS_FLAGS(IsAUState);
  PARSE_CLASS_FLAGS(IsAUFunction);
  PARSE_CLASS_FLAGS(IsAUStructProperty);
  if (s.Size())
  {
    s = s.Substr(0, s.Size() - 2);
  }
  else
  {
    s = "None";
  }
  return s;
}

FString TextureCompressionSettingsToString(uint8 flags)
{
  FString result;
  switch (flags)
  {
  default:
  case TC_Default:
    result = "TC_Default";
    break;
  case TC_Normalmap:
    result = "TC_Normalmap";
    break;
  case TC_Displacementmap:
    result = "TC_Displacementmap";
    break;
  case TC_NormalmapAlpha:
    result = "TC_NormalmapAlpha";
    break;
  case TC_Grayscale:
    result = "TC_Grayscale";
    break;
  case TC_HighDynamicRange:
    result = "TC_HighDynamicRange";
    break;
  case TC_OneBitAlpha:
    result = "TC_OneBitAlpha";
    break;
  case TC_NormalmapUncompressed:
    result = "TC_NormalmapUncompressed";
    break;
  case TC_NormalmapBC5:
    result = "TC_NormalmapBC5";
    break;
  case TC_OneBitMonochrome:
    result = "TC_OneBitMonochrome";
    break;
  case TC_SimpleLightmapModification:
    result = "TC_SimpleLightmapModification";
    break;
  case TC_VectorDisplacementmap:
    result = "TC_VectorDisplacementmap";
    break;
  case TC_MAX:
    result = "MAX";
    break;
  }
  return result;
}

std::string GetAppVersion()
{
  std::stringstream stream;
  stream << "v." << std::fixed << std::setprecision(2) << APP_VER << BUILD_SUFFIX;
  return stream.str();
}

bool HasAVX2()
{
  static bool result = _HasAVX2();
  return result;
}

#if _DEBUG
void DumpData(void* data, int size, const char* path)
{
  std::string p = std::string(DUMP_PATH) + "\\" + path;
  std::ofstream s(p, std::ios::out | std::ios::binary);
  s.write((const char*)data, size);
}
#endif