// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/logger.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up::shell {
    class LogWindow {
    public:
        LogWindow(Logger& logger);
        ~LogWindow();

        LogWindow(LogWindow const&) = delete;
        LogWindow& operator=(LogWindow const&) = delete;

        void draw();

        bool isOpen() const noexcept { return _open; }
        bool open(bool state = true) noexcept { return _open = state; }

    private:
        class LogWindowSink;
        struct LogEntry {
            LogSeverity severity = LogSeverity::Info;
            string message;
            string location;
            size_t count = 1;
        };

        Logger& _logger;
        rc<LogWindowSink> _receiver;
        vector<LogEntry> _logs;
        LogSeverityMask _mask = LogSeverityMask::Everything;
        bool _open = true;
        bool _stickyBottom = true;
        char _filter[128] = {
            0,
        };

        friend LogWindowSink;
    };
} // namespace up::shell
