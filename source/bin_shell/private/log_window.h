// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/logger.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up::shell {
    struct LogEntry {
        LogSeverity severity = LogSeverity::Info;
        string message;
        string category;
        string location;
        size_t count = 1;
    };

    class LogHistory {
    public:
        LogHistory();
        ~LogHistory();

        view<LogEntry> logs() const noexcept { return _logs; }

    private:
        class LogHistorySink;

        rc<LogHistorySink> _sink;
        vector<LogEntry> _logs;

        friend LogHistorySink;
    };

    class LogWindow {
    public:
        explicit LogWindow(LogHistory& history) : _history(history) {}

        LogWindow(LogWindow const&) = delete;
        LogWindow& operator=(LogWindow const&) = delete;

        void draw();

        bool isOpen() const noexcept { return _open; }
        bool open(bool state = true) noexcept { return _open = state; }

    private:
        LogHistory& _history;
        LogSeverityMask _mask = LogSeverityMask::Everything;
        bool _open = false;
        bool _stickyBottom = true;
        char _filter[128] = {
            0,
        };
    };
} // namespace up::shell
