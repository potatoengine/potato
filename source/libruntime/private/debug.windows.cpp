// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "debug.h"
#include "callstack.h"
#include "debug.windows.h"

#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/platform_windows.h"
#include "potato/spud/string_format.h"

namespace {
    struct DialogData {
        char const* message = nullptr;
        char const* location = nullptr;
        char const* condition = nullptr;
        char const* callstack = nullptr;
    };

    void CopyToClipboard(DialogData const& data) noexcept {
        constexpr int clipboard_size_bytes = 1024;

        // copy into clipboard
        if (OpenClipboard(nullptr) == TRUE) {
            EmptyClipboard();

            up::fixed_string_writer<clipboard_size_bytes> buffer;
            up::format_append(
                buffer,
                "ASSERTION FAILED: {}\r\n{}\r\n{}\r\n{}",
                data.condition,
                data.message,
                data.location,
                data.callstack);

            if (HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, buffer.size() + 1)) {
                void* lockedData = GlobalLock(handle);
                if (lockedData != nullptr) {
                    std::memcpy(lockedData, buffer.data(), buffer.size());
                    GlobalUnlock(handle);

                    SetClipboardData(CF_TEXT, handle);
                }
            }
            CloseClipboard();
        }
    }

    INT_PTR CALLBACK AssertDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        using namespace up;

        switch (msg) {
            case WM_INITDIALOG: {
                // force the dialog to show
                ShowWindow(hwnd, TRUE);

                // save this for the copy button
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, lparam);

                // style overrides that we can't set in the dialog template in the .rc file
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                HICON icon = LoadIconW(nullptr, IDI_ERROR);
                SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
                DestroyIcon(icon);

                // initialize text fields
                DialogData const& data = *reinterpret_cast<DialogData const*>(lparam);
                SetDlgItemTextA(hwnd, IDC_MESSAGE_TEXT, data.message);
                SetDlgItemTextA(hwnd, IDC_LOCATION_TEXT, data.location);
                SetDlgItemTextA(hwnd, IDC_ASSERT_TEXT, data.condition);
                SetDlgItemTextA(hwnd, IDC_CALLSTACK_TEXT, data.callstack);
                break;
            }
            case WM_COMMAND:
                switch (LOWORD(wparam)) {
                    case IDABORT:
                        EndDialog(hwnd, INT_PTR(_detail::FatalErrorAction::Abort));
                        return TRUE;
                    case IDBREAK:
                        EndDialog(hwnd, INT_PTR(_detail::FatalErrorAction::BreakInDebugger));
                        return TRUE;
                    case IDIGNORE:
                        EndDialog(hwnd, INT_PTR(_detail::FatalErrorAction::IgnoreOnce));
                        return TRUE;
                    case IDIGNORE_ALWAYS:
                        EndDialog(hwnd, INT_PTR(_detail::FatalErrorAction::IgnoreAlways));
                        return TRUE;
                    case IDCOPY:
                        CopyToClipboard(*reinterpret_cast<DialogData const*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
                        break;
                }
                break;
            case WM_CLOSE:
                // if the user just tries to close the dialog, assume that they're trying to ignore the message
                EndDialog(hwnd, INT_PTR(_detail::FatalErrorAction::IgnoreOnce));
                return TRUE;
        }
        return FALSE;
    }
} // namespace

namespace up::_detail {
    auto handleFatalError(
        char const* file,
        int line,
        char const* failedConditionText,
        char const* messageText,
        char const* callstackText) -> FatalErrorAction {
        constexpr int location_buffer_bytes = 128;
        fixed_string_writer<location_buffer_bytes> location_buffer;
        format_append(location_buffer, "{}({})", file, line);

        DialogData data;
        data.message = messageText;
        data.condition = failedConditionText;
        data.location = location_buffer.c_str();
        data.callstack = callstackText;

        // display dialog
        HMODULE module = nullptr;
        static const LPCWSTR address = L"";
        GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            address,
            &module);
        INT_PTR rs = DialogBoxParamW(
            module,
            MAKEINTRESOURCEW(IDD_ASSERT), // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            GetActiveWindow(),
            static_cast<DLGPROC>(AssertDialogProc),
            reinterpret_cast<LPARAM>(&data));
        return FatalErrorAction(rs);
    }
} // namespace up::_detail
