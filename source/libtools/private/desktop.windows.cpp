// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "desktop.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/platform_windows.h"

#define STRICT_TYPED_ITEMIDS
#include <Shlobj.h>
#include <shellapi.h>

bool up::desktop::openInExternalEditor(zstring_view filename) {
    SHELLEXECUTEINFOA info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.lpFile = filename.c_str();
    info.lpVerb = "edit";
    info.fMask = SEE_MASK_FLAG_NO_UI;
    info.nShow = TRUE;
    return ShellExecuteExA(&info) == TRUE;
}

bool up::desktop::openInBrowser(zstring_view url) {
    SHELLEXECUTEINFOA info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.lpFile = url.c_str();
    info.lpVerb = "open";
    info.fMask = SEE_MASK_FLAG_NO_UI;
    info.nShow = TRUE;
    return ShellExecuteExA(&info) == TRUE;
}

bool up::desktop::openInExplorer(zstring_view folder) {
    SHELLEXECUTEINFOA info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.lpFile = folder.c_str();
    info.lpVerb = "open";
    info.fMask = SEE_MASK_FLAG_NO_UI;
    info.nShow = TRUE;
    return ShellExecuteExA(&info) == TRUE;
}

bool up::desktop::selectInExplorer(zstring_view filename) {
    PIDLIST_ABSOLUTE pidl = nullptr;
    SFGAOF flags = 0;

    wchar_t widePath[2048] = {};

    MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), filename.size(), widePath, sizeof(widePath));
    SHParseDisplayName(widePath, nullptr, &pidl, 0, &flags);
    SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
    ILFree(pidl);
    return true;
}
