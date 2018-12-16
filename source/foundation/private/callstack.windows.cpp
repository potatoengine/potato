// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#include "callstack.h"
#include "platform.h"

#if !defined(GM_PLATFORM_WINDOWS)
#    error "Unsupported platform"
#endif

#include "numeric_util.h"
#include "platform_windows.h"
#include "string.h"


#pragma warning(disable : 4091)
#include <dbghelp.h>

namespace {

    class CallstackHelper {
    public:
        CallstackHelper();
        ~CallstackHelper();

        CallstackHelper(CallstackHelper const&) = delete;
        CallstackHelper& operator=(CallstackHelper const&) = delete;

        bool isInitialized() const { return _initialized; }
        int captureStackTrace(int skip, int count, void** entries);
        void readSymbol(void* entry, PSYMBOL_INFO symInfo, PIMAGEHLP_LINE64 lineInfo);

        static CallstackHelper& instance();

    private:
        bool _initialized = false;
        HANDLE _process;

        using CaptureStackBackTraceType = USHORT(WINAPI*)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);

        HMODULE _kernelLib;
        CaptureStackBackTraceType _captureStackBackTrace;

#if !defined(NDEBUG)
        using SymInitializeType = BOOL(WINAPI*)(__in HANDLE, __in_opt PCTSTR, __in BOOL);
        using SymCleanupType = BOOL(WINAPI*)(__in HANDLE);
        using SymGetLineFromAddr64Type = BOOL(WINAPI*)(__in HANDLE, __in DWORD64, __out PDWORD, __out PIMAGEHLP_LINE64);
        using SymFromAddrType = BOOL(WINAPI*)(__in HANDLE, __in DWORD64, __out_opt PDWORD64, __inout PSYMBOL_INFO);

        HMODULE _dbghelpLib;
        SymInitializeType _symInitialize;
        SymCleanupType _symCleanup;
        SymGetLineFromAddr64Type _symGetLineFromAddr64;
        SymFromAddrType _symFromAddr;
#endif // !defined(NDEBUG)
    };

    CallstackHelper::CallstackHelper() {
        _process = GetCurrentProcess();

        _kernelLib = LoadLibraryW(L"kernel32.dll");
        if (!_kernelLib)
            return;

        _captureStackBackTrace = reinterpret_cast<CaptureStackBackTraceType>(GetProcAddress(_kernelLib, "RtlCaptureStackBackTrace"));
        if (!_captureStackBackTrace)
            return;

#if !defined(NDEBUG)
        _dbghelpLib = LoadLibraryW(L"dbghelp.dll");
        if (!_dbghelpLib)
            return;

        _symInitialize = reinterpret_cast<SymInitializeType>(GetProcAddress(_dbghelpLib, "SymInitialize"));
        if (!_symInitialize)
            return;

        _symCleanup = reinterpret_cast<SymCleanupType>(GetProcAddress(_dbghelpLib, "SymCleanup"));
        if (!_symCleanup)
            return;

        _symGetLineFromAddr64 = reinterpret_cast<SymGetLineFromAddr64Type>(GetProcAddress(_dbghelpLib, "SymGetLineFromAddr64"));
        if (!_symGetLineFromAddr64)
            return;

        _symFromAddr = reinterpret_cast<SymFromAddrType>(GetProcAddress(_dbghelpLib, "SymFromAddr"));
        if (!_symFromAddr)
            return;

        if (!_symInitialize(_process, nullptr, true))
            return;
#endif // !defined(NDEBUG)

        _initialized = true;
    }

    CallstackHelper::~CallstackHelper() {

#if !defined(NDEBUG)
        if (_symCleanup != nullptr)
            _symCleanup(_process);

        _symCleanup = nullptr;
        _symInitialize = nullptr;
        _symGetLineFromAddr64 = nullptr;
        _symFromAddr = nullptr;

        FreeLibrary(_dbghelpLib);
#endif // !defined(NDEBUG)

        _captureStackBackTrace = nullptr;
        FreeLibrary(_kernelLib);
    }

    int CallstackHelper::captureStackTrace(int skip, int count, void** entries) {
        if (_captureStackBackTrace != nullptr)
            return _captureStackBackTrace(skip + 1, count, entries, nullptr);
        else
            return 0;
    }

#if !defined(NDEBUG)
    void CallstackHelper::readSymbol(void* entry, PSYMBOL_INFO symInfo, PIMAGEHLP_LINE64 lineInfo) {
        if (_symFromAddr != nullptr)
            _symFromAddr(GetCurrentProcess(), DWORD64(entry), nullptr, symInfo);

        DWORD displacement = 0;
        if (_symGetLineFromAddr64 != nullptr)
            _symGetLineFromAddr64(GetCurrentProcess(), DWORD64(entry), &displacement, lineInfo);
    }
#endif // !defined(NDEBUG)

    CallstackHelper& CallstackHelper::instance() {
        static CallstackHelper helper;
        return helper;
    }

} // anonymous namespace

int gm::CallStackReader::readCallstack(array_view<uintptr> addresses, int skip) {
    CallstackHelper& helper = CallstackHelper::instance();

    if (!helper.isInitialized())
        return 0;

    return helper.captureStackTrace(skip + 1, static_cast<int>(addresses.size()), reinterpret_cast<void**>(addresses.data()));
}

bool gm::CallStackReader::tryResolveCallstack(array_view<uintptr const> addresses, array_view<CallStackRecord> out_records) {
#if !defined(NDEBUG)
    CallstackHelper& helper = CallstackHelper::instance();

    if (!helper.isInitialized())
        return false;

    int const max = static_cast<int>(gm::min(addresses.size(), out_records.size()));

    int constexpr kMaxNameLen = 128;

    std::aligned_union_t<sizeof(SYMBOL_INFO) + kMaxNameLen, SYMBOL_INFO> symbolStorage;
    PSYMBOL_INFO symbolInfoPtr = reinterpret_cast<PSYMBOL_INFO>(&symbolStorage);
    IMAGEHLP_LINE64 imagehlpLine64;

    ZeroMemory(&symbolStorage, sizeof(symbolStorage));
    ZeroMemory(&imagehlpLine64, sizeof(imagehlpLine64));

    symbolInfoPtr->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbolInfoPtr->MaxNameLen = kMaxNameLen;

    HANDLE process = GetCurrentProcess();

    for (auto index = 0; index != max; ++index) {
        helper.readSymbol(reinterpret_cast<void*>(addresses[index]), symbolInfoPtr, &imagehlpLine64);

        CallStackRecord& record = out_records[index];
        record.symbol = string_view(symbolInfoPtr->Name, symbolInfoPtr->NameLen);
        record.filename = imagehlpLine64.FileName;
        record.line = imagehlpLine64.LineNumber;

        // Disable for now, this makes "prettier" traces, but not necessarily more useful ones
        //string_span filename(imagehlpLine64.FileName);
        //auto const pos = find_last_of(filename, "/\\");
        //if (pos != string_span::npos)
        //	entry.filename = string_span(filename.begin() + pos + 1, filename.end());
    }

    return true;
#else
    return false;
#endif // !defined(NDEBUG)
}
