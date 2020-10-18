// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"

#include "potato/spud/zstring_view.h"

extern "C" {
struct sqlite3;
struct sqlite3_stmt;
}

namespace up {
    inline namespace posql {
        enum class SqlResult { Ok = 0, Error };

        class Database;
        class Statement;
        class Query;
        class Row;

        class Database {
        public:
            Database() noexcept = default;
            ~Database() noexcept { close(); }

            Database(Database const&) = delete;
            Database& operator=(Database const&) = delete;

            UP_POSQL_API [[nodiscard]] SqlResult open(zstring_view file_name) noexcept;
            UP_POSQL_API void close() noexcept;

            [[nodiscard]] bool empty() const noexcept { return _conn == nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return _conn != nullptr; }

            UP_POSQL_API [[nodiscard]] Statement prepare(zstring_view sql) noexcept;
            UP_POSQL_API [[nodiscard]] SqlResult execute(zstring_view sql) noexcept;

        private:
            sqlite3* _conn = nullptr;
        };

        class Statement {
        public:
            Statement() noexcept = default;
            explicit Statement(sqlite3_stmt* stmt) noexcept : _stmt(stmt) {}
            UP_POSQL_API ~Statement() noexcept;

            Statement(Statement const&) = delete;
            Statement& operator=(Statement const&) = delete;

            [[nodiscard]] bool empty() const noexcept { return _stmt == nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return _stmt != nullptr; }

            UP_POSQL_API [[nodiscard]] SqlResult execute() noexcept;
            UP_POSQL_API [[nodiscard]] Query query() noexcept;

            UP_POSQL_API SqlResult bind(int index, int64 value) noexcept;
            UP_POSQL_API SqlResult bind(int index, zstring_view value) noexcept;

        private:
            sqlite3_stmt* _stmt = nullptr;
        };

        class Query {
        public:
            using value_type = Row;
            class iterator;
            struct sentinel {};

            ~Query() noexcept = default;

            inline iterator begin() noexcept;
            sentinel end() noexcept { return {}; }

        private:
            explicit Query(sqlite3_stmt* stmt) : _stmt(stmt) {}

            sqlite3_stmt* _stmt = nullptr;

            friend iterator;
            friend Statement;
        };

        class Query::iterator {
        public:
            UP_POSQL_API [[nodiscard]] bool operator==(Query::sentinel) noexcept;
            UP_POSQL_API iterator& operator++() noexcept;
            UP_POSQL_API [[nodiscard]] Row operator*() noexcept;

        private:
            explicit iterator(sqlite3_stmt* stmt) : _stmt(stmt) {}

            sqlite3_stmt* _stmt = nullptr;

            friend Query;
        };

        Query::iterator Query::begin() noexcept { return iterator(_stmt); }

        class Row {
        public:
            UP_POSQL_API [[nodiscard]] int64 get_int64(int index) noexcept;
            UP_POSQL_API [[nodiscard]] zstring_view get_string(int index) noexcept;

        private:
            explicit Row(sqlite3_stmt* stmt) : _stmt(stmt) {}

            friend Query::iterator;

            sqlite3_stmt* _stmt = nullptr;
        };

    } // namespace posql
} // namespace up
