// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"

#include "potato/spud/unique_resource.h"
#include "potato/spud/zstring_view.h"

extern "C" {
struct sqlite3;
struct sqlite3_stmt;
int sqlite3_close_v2(sqlite3*);
int sqlite3_finalize(sqlite3_stmt*);
}

namespace up {
    inline namespace posql {

        using SqliteConnection = up::unique_resource<sqlite3*, sqlite3_close_v2>;
        using SqliteStatement = up::unique_resource<sqlite3_stmt*, sqlite3_finalize>;

        enum class SqlResult { Ok = 0, Error };

        class Database;
        class Statement;

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

        private:
            SqliteConnection _conn;
        };

        class Statement {
        public:
            Statement() noexcept = default;
            explicit Statement(SqliteStatement stmt) noexcept : _stmt(std::move(stmt)) {}
            UP_POSQL_API ~Statement() noexcept;

            Statement(Statement const&) = delete;
            Statement& operator=(Statement const&) = delete;

            [[nodiscard]] bool empty() const noexcept { return _stmt == nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return _stmt != nullptr; }

            UP_POSQL_API [[nodiscard]] SqlResult execute() noexcept;

        private:
            SqliteStatement _stmt;
        };

    } // namespace posql
} // namespace up
