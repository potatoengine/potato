#include <Windows.h>

namespace AK {
    void* AllocHook(size_t in_size) {
        return malloc(in_size);
    }
    void FreeHook(void* in_ptr) {
        free(in_ptr);
    }
    void* VirtualAllocHook(
        void* in_pMemAddress,
        size_t in_size,
        DWORD in_dwAllocationType,
        DWORD in_dwProtect) {
        return VirtualAlloc(in_pMemAddress, in_size, in_dwAllocationType, in_dwProtect);
    }
    void VirtualFreeHook(
        void* in_pMemAddress,
        size_t in_size,
        DWORD in_dwFreeType) {
        VirtualFree(in_pMemAddress, in_size, in_dwFreeType);
    }
} // namespace AK
