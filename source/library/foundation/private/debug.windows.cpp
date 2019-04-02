// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "debug.windows.h"
#include "callstack.h"
#include "debug.h"
#include "platform_windows.h"
#include "string_format.h"
#include "fixed_string_writer.h"

namespace {
    struct DialogData {
        char const* message = nullptr;
        char const* location = nullptr;
        char const* condition = nullptr;
        char const* callstack = nullptr;
    };

    void CopyToClipboard(DialogData const& data) {
        // copy into clipboard
        if (OpenClipboard(nullptr) == TRUE) {
            EmptyClipboard();

            gm::fixed_string_writer<1024> buffer;
            gm::format_into(buffer, "ASSERTION FAILED: {}\r\n{}\r\n{}\r\n{}", data.condition, data.message, data.location, data.callstack);

            if (HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, buffer.size() + 1)) {
                void* data = GlobalLock(handle);
                std::memcpy(data, buffer.data(), buffer.size());
                GlobalUnlock(handle);

                SetClipboardData(CF_TEXT, handle);
            }
            CloseClipboard();
        }
    }

    INT_PTR CALLBACK AssertDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        using namespace gm;

        switch (msg) {
        case WM_INITDIALOG: {
            // save this for the copy button
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, lparam);

            // style overrides that we can't set in the dialog template in the .rc file
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
                EndDialog(hwnd, INT_PTR(error_action::abort));
                return TRUE;
            case IDBREAK:
                EndDialog(hwnd, INT_PTR(error_action::debugger_break));
                return TRUE;
            case IDIGNORE:
                EndDialog(hwnd, INT_PTR(error_action::ignore_once));
                return TRUE;
            case IDIGNORE_ALWAYS:
                EndDialog(hwnd, INT_PTR(error_action::ignore_always));
                return TRUE;
            case IDCOPY:
                CopyToClipboard(*reinterpret_cast<DialogData const*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
                break;
            }
            break;
        case WM_CLOSE:
            // if the user just tries to close the dialog, assume that they're trying to ignore the message
            EndDialog(hwnd, INT_PTR(error_action::ignore_once));
            return TRUE;
        }
        return FALSE;
    }

    GM_NOINLINE gm::error_action ShowAssertDialog(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText) {
        using namespace gm;

        fixed_string_writer<128> location_buffer;
        format_into(location_buffer, "{}({})", file, line);

        DialogData data;
        data.message = messageText;
        data.condition = failedConditionText;
        data.location = location_buffer.c_str();
        data.callstack = callstackText;

        // display dialog
        HMODULE module = nullptr;
        static const LPCWSTR address = L"";
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, address, &module);
        INT_PTR rs = DialogBoxParamW(module, MAKEINTRESOURCEW(IDD_ASSERT), GetActiveWindow(), static_cast<DLGPROC>(AssertDialogProc), reinterpret_cast<LPARAM>(&data));
        return error_action(rs);
    }
} // namespace

namespace gm::_detail {
    auto platform_fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText) -> error_action {
        return ShowAssertDialog(file, line, failedConditionText, messageText, callstackText);
    }
} // namespace gm::_detail
