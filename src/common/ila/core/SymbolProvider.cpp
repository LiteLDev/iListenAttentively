/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 *
 * This file is derived from Microsoft implementation file delayhlp.cpp, which
 * is free for users to modify and derive.
 *
 * Modified By BDSLiteloader Developers in ordered to support SymDB
 * You Must Link with BDSLiteLoader.dll to get dlsym support
 *
 * To use this Method, you should static link this project (and LiteLoader)
 * Open Your Plugin's Project Setting
 * [Properties -> Linker -> Input -> Delay Load DLL] and add "bedrock_server.dll" to the list
 * then make sure you link bedrock_server_api.lib and bedrock_server_var.lib and here we go!
 *
 * BDS Funtions will be loaded as you call them
 *
 */

#pragma warning(disable : 4267)

#include <cstdint>
#include <cstdio>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <delayimp.h>

constexpr auto BDSAPI_FAKEDLL_NAME = "bedrock_runtime.dll";

inline PCImgDelayDescr pImgDelayDescr_BDS = nullptr;

// LiteLoader Api to Fetch Function Address
namespace ll::memory {
using FuncPtr = void*;
extern FuncPtr resolveSymbol(const char* symbol);
} // namespace ll::memory

using namespace ll::memory;

static size_t __strlen(const char* sz) {
    const char* szEnd = sz;
    while (*szEnd++ != 0) { ; }
    return szEnd - sz - 1;
}

static int __memcmp(const void* pv1, const void* pv2, size_t cb) {
    if (!cb) return 0;
    while (--cb && *(char*)pv1 == *(char*)pv2) {
        pv1 = ((char*)pv1) + 1;
        pv2 = ((char*)pv2) + 1;
    }
    return *((unsigned char*)pv1) - *((unsigned char*)pv2);
}

static void* __memcpy(void* pvDst, const void* pvSrc, size_t cb) {
    void* pvRet = pvDst;
    while (cb--) {
        *(char*)pvDst = *(char*)pvSrc;
        pvDst         = ((char*)pvDst) + 1;
        pvSrc         = ((char*)pvSrc) + 1;
    }
    return pvRet;
}

static unsigned IndexFromPImgThunkData(PCImgThunkData pitdCur, PCImgThunkData pitdBase) {
    return (unsigned)(pitdCur - pitdBase);
}

ExternC IMAGE_DOS_HEADER __ImageBase;

template <class X>
constexpr X PFromRva(RVA rva) {
    return X(PBYTE(&__ImageBase) + rva);
}

typedef struct UnloadInfo* PUnloadInfo;
typedef struct UnloadInfo {
    PUnloadInfo     puiNext;
    PCImgDelayDescr pidd;
} UnloadInfo;

static unsigned CountOfImports(PCImgThunkData pitdBase) {
    unsigned       cRet = 0;
    PCImgThunkData pitd = pitdBase;
    while (pitd->u1.Function) {
        pitd++;
        cRet++;
    }
    return cRet;
}

PUnloadInfo __puiHead = nullptr;

static UnloadInfo* add_ULI(PCImgDelayDescr pidd_) {
    auto* ret    = (UnloadInfo*)LocalAlloc(LPTR, sizeof(UnloadInfo));
    ret->pidd    = pidd_;
    ret->puiNext = __puiHead;
    __puiHead    = ret;
    return ret;
}

static void del_ULI(UnloadInfo* p) {
    if (p) {
        PUnloadInfo* ppui = &__puiHead;
        while (*ppui && *ppui != p) { ppui = &((*ppui)->puiNext); }
        if (*ppui == p) *ppui = p->puiNext;
        LocalFree((void*)p);
    }
}

typedef struct InternalImgDelayDescr {
    DWORD          grAttrs;
    LPCSTR         szName;
    HMODULE*       phmod;
    PImgThunkData  pIAT;
    PCImgThunkData pINT;
    PCImgThunkData pBoundIAT;
    PCImgThunkData pUnloadIAT;
    DWORD          dwTimeStamp;
} InternalImgDelayDescr;

typedef InternalImgDelayDescr*       PIIDD;
typedef const InternalImgDelayDescr* PCIIDD;

static PIMAGE_NT_HEADERS WINAPI PinhFromImageBase(HMODULE hmod) {
    return (PIMAGE_NT_HEADERS)(((PBYTE)(hmod)) + ((PIMAGE_DOS_HEADER)(hmod))->e_lfanew);
}

static void WINAPI OverlayIAT(PImgThunkData pitdDst, PCImgThunkData pitdSrc) {
    __memcpy(pitdDst, pitdSrc, CountOfImports(pitdDst) * sizeof(IMAGE_THUNK_DATA));
}

static DWORD WINAPI TimeStampOfImage(PIMAGE_NT_HEADERS pinh) { return pinh->FileHeader.TimeDateStamp; }

static int WINAPI FLoadedAtPreferredAddress(PIMAGE_NT_HEADERS pinh, HMODULE hmod) {
    return ((UINT_PTR)(hmod)) == pinh->OptionalHeader.ImageBase;
}

ExternC FARPROC WINAPI __delayLoadHelper2(PCImgDelayDescr pidd, FARPROC* ppfnIATEntry) {

    InternalImgDelayDescr idd = {
        pidd->grAttrs,
        PFromRva<LPCSTR>(pidd->rvaDLLName),
        PFromRva<HMODULE*>(pidd->rvaHmod),
        PFromRva<PImgThunkData>(pidd->rvaIAT),
        PFromRva<PCImgThunkData>(pidd->rvaINT),
        PFromRva<PCImgThunkData>(pidd->rvaBoundIAT),
        PFromRva<PCImgThunkData>(pidd->rvaUnloadIAT),
        pidd->dwTimeStamp
    };
    DelayLoadInfo dli = {
        sizeof(DelayLoadInfo),
        pidd,
        ppfnIATEntry,
        idd.szName,
        {0, {nullptr}},
        nullptr,
        nullptr,
        0,
    };
    HMODULE        hmod;
    unsigned       iIAT, iINT;
    PCImgThunkData pitd;
    FARPROC        pfnRet;

    if (!(idd.grAttrs & dlattrRva)) {
        PDelayLoadInfo rgpdli[1] = {&dli};
        RaiseException(VcppException(ERROR_SEVERITY_ERROR, ERROR_INVALID_PARAMETER), 0, 1, (PULONG_PTR)(rgpdli));
        return nullptr;
    }
    hmod = *idd.phmod;

    // Calculate the index for the IAT entry in the import address table
    // N.B. The INT entries are ordered the same as the IAT entries so
    // the calculation can be done on the IAT side.
    iIAT = IndexFromPImgThunkData((PCImgThunkData)(ppfnIATEntry), idd.pIAT);
    iINT = iIAT;
    pitd = &(idd.pINT[iINT]);

    dli.dlp.fImportByName = !IMAGE_SNAP_BY_ORDINAL(pitd->u1.Ordinal);
    if (dli.dlp.fImportByName)
        dli.dlp.szProcName =
            (LPCSTR)((PFromRva<PIMAGE_IMPORT_BY_NAME>((RVA)((UINT_PTR)(pitd->u1.AddressOfData))))->Name);
    else dli.dlp.dwOrdinal = (DWORD)(IMAGE_ORDINAL(pitd->u1.Ordinal));
    pfnRet = nullptr;
    if (__pfnDliNotifyHook2) {
        pfnRet = ((*__pfnDliNotifyHook2)(dliStartProcessing, &dli));
        if (pfnRet != nullptr) goto HookBypass;
    }
    if (!pImgDelayDescr_BDS && !strcmp(dli.szDll, BDSAPI_FAKEDLL_NAME)) { pImgDelayDescr_BDS = pidd; }

    if (pImgDelayDescr_BDS == pidd) {
        auto length = __strlen(dli.dlp.szProcName) + 1;
        auto symbol = new char[length];
        __memcpy(symbol, dli.dlp.szProcName, length);
        constexpr uint8_t mXorKey = 0;
        for (size_t i = 0; i < length; ++i) symbol[i] ^= mXorKey;
        symbol[length - 1] = '\0';
        pfnRet = (FARPROC)resolveSymbol(symbol);
        delete[] symbol;
        goto SetEntryHookBypass;
    }

    if (hmod == nullptr) {
        if (__pfnDliNotifyHook2) hmod = (HMODULE)(((*__pfnDliNotifyHook2)(dliNotePreLoadLibrary, &dli)));
        if (hmod == nullptr) hmod = LoadLibraryA(dli.szDll);
        if (hmod == nullptr) {
            dli.dwLastError = GetLastError();
            if (__pfnDliFailureHook2) hmod = (HMODULE)((*__pfnDliFailureHook2)(dliFailLoadLib, &dli));
            if (hmod == nullptr) {
                PDelayLoadInfo rgpdli[1] = {&dli};
                RaiseException(VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND), 0, 1, (PULONG_PTR)(rgpdli));
                return dli.pfnCur;
            }
        }
        auto hmodT = (HMODULE)(InterlockedExchangePointer((PVOID*)idd.phmod, (PVOID)(hmod)));
        if (hmodT != hmod) {
            if (pidd->rvaUnloadIAT) add_ULI(pidd);
        } else FreeLibrary(hmod);
    }
    dli.hmodCur = hmod;
    if (__pfnDliNotifyHook2) pfnRet = (*__pfnDliNotifyHook2)(dliNotePreGetProcAddress, &dli);
    if (pfnRet == nullptr) {
        if (pidd->rvaBoundIAT && pidd->dwTimeStamp) {
            auto pinh = (PIMAGE_NT_HEADERS)(PinhFromImageBase(hmod));
            if (pinh->Signature == IMAGE_NT_SIGNATURE && TimeStampOfImage(pinh) == idd.dwTimeStamp &&
                FLoadedAtPreferredAddress(pinh, hmod)) {
                pfnRet = (FARPROC)((UINT_PTR)(idd.pBoundIAT[iIAT].u1.Function));
                if (pfnRet != nullptr) goto SetEntryHookBypass;
            }
        }
        pfnRet = GetProcAddress(hmod, dli.dlp.szProcName);
    }
    if (!pfnRet) {
        dli.dwLastError = GetLastError();
        if (__pfnDliFailureHook2) pfnRet = (*__pfnDliFailureHook2)(dliFailGetProc, &dli);
        if (!pfnRet) {
            PDelayLoadInfo rgpdli[1] = {&dli};
            RaiseException(VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND), 0, 1, (PULONG_PTR)(rgpdli));
            pfnRet = dli.pfnCur;
        }
    }
SetEntryHookBypass:
    *ppfnIATEntry = pfnRet;
HookBypass:
    if (__pfnDliNotifyHook2) {
        dli.dwLastError = 0;
        dli.hmodCur     = hmod;
        dli.pfnCur      = pfnRet;
        (*__pfnDliNotifyHook2)(dliNoteEndProcessing, &dli);
    }
    return pfnRet;
}

BOOL WINAPI __FUnloadDelayLoadedDLL2(LPCSTR szDll) {

    BOOL        fRet = FALSE;
    PUnloadInfo pui  = __puiHead;

    for (pui = __puiHead; pui; pui = pui->puiNext) {
        auto   szName = PFromRva<LPCSTR>(pui->pidd->rvaDLLName);
        size_t cbName = __strlen(szName);
        if (cbName == __strlen(szDll) && __memcmp(szDll, szName, cbName) == 0) break;
    }
    if (pui && pui->pidd->rvaUnloadIAT) {
        PCImgDelayDescr pidd  = pui->pidd;
        auto            phmod = PFromRva<HMODULE*>(pidd->rvaHmod);
        HMODULE         hmod  = *phmod;
        OverlayIAT(PFromRva<PImgThunkData>(pidd->rvaIAT), PFromRva<PCImgThunkData>(pidd->rvaUnloadIAT));
        FreeLibrary(hmod);
        *phmod = nullptr;
        del_ULI((UnloadInfo*)pui);
        fRet = TRUE;
    }
    return fRet;
}

HRESULT WINAPI __HrLoadAllImportsForDll(LPCSTR szDll) {
    HRESULT           hrRet = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
    PIMAGE_NT_HEADERS pinh  = PinhFromImageBase((HMODULE)(&__ImageBase));
    if (pinh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size) {
        auto pidd = PFromRva<PCImgDelayDescr>(
            pinh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress
        );
        while (pidd->rvaDLLName) {
            auto   szDllCur  = PFromRva<LPCSTR>(pidd->rvaDLLName);
            size_t cchDllCur = __strlen(szDllCur);
            if (cchDllCur == __strlen(szDll) && __memcmp(szDll, szDllCur, cchDllCur) == 0) break;
            pidd++;
        }
        if (pidd->rvaDLLName) {
            auto     ppfnIATEntry    = PFromRva<FARPROC*>(pidd->rvaIAT);
            size_t   cpfnIATEntries  = CountOfImports((PCImgThunkData)(ppfnIATEntry));
            FARPROC* ppfnIATEntryMax = ppfnIATEntry + cpfnIATEntries;
            for (; ppfnIATEntry < ppfnIATEntryMax; ppfnIATEntry++) { __delayLoadHelper2(pidd, ppfnIATEntry); }
            hrRet = S_OK;
        }
    }
    return hrRet;
}

struct DynamicInitializer {
    DynamicInitializer() { __HrLoadAllImportsForDll(BDSAPI_FAKEDLL_NAME); }
};

#pragma warning(disable : 4073)
#pragma init_seg(lib)

ExternC const PfnDliHook __pfnDliNotifyHook2 = nullptr; // NOLINT(misc-misplaced-const)

ExternC const PfnDliHook __pfnDliFailureHook2 = nullptr; // NOLINT(misc-misplaced-const)

inline DynamicInitializer dynamicInitializer;
