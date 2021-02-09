// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "posql.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/string_format.h"

#include <sqlite3.h>

up::SqlResult up::Database::open(zstring_view file_name) noexcept {
    close();

    auto const rs = sqlite3_open(file_name.c_str(), &_conn);
    if (rs != SQLITE_OK) {
        return SqlResult::Error;
    }

    return SqlResult::Ok;
}

void up::Database::close() noexcept {
    sqlite3_close(_conn);
    _conn = nullptr;
}

up::Statement up::Database::prepare(zstring_view sql) noexcept {
    UP_ASSERT(_conn != nullptr);
    sqlite3_stmt* stmt = nullptr;
    [[maybe_unused]] auto const rs = sqlite3_prepare_v3(_conn, sql.c_str(), -1, 0, &stmt, nullptr);
    UP_ASSERT(rs == SQLITE_OK);
    return Statement(stmt);
}

up::SqlResult up::Database::execute(zstring_view sql) noexcept {
    UP_ASSERT(_conn != nullptr);
    auto const rs = sqlite3_exec(_conn, sql.c_str(), nullptr, nullptr, nullptr);
    return rs == SQLITE_OK ? SqlResult::Ok : SqlResult::Error;
}

up::Transaction up::Database::begin() noexcept {
    UP_ASSERT(_conn != nullptr);
    if (execute("BEGIN") != SqlResult::Ok) {
        return Transaction(nullptr);
    }
    return Transaction(_conn);
}

void up::Transaction::commit() {
    if (_conn != nullptr) {
        sqlite3_exec(_conn, "COMMIT", nullptr, nullptr, nullptr);
        _conn = nullptr;
    }
}

void up::Transaction::rollback() {
    if (_conn != nullptr) {
        sqlite3_exec(_conn, "ROLLBACK", nullptr, nullptr, nullptr);
        _conn = nullptr;
    }
}

up::Statement::~Statement() noexcept {
    sqlite3_finalize(_stmt);
}

up::Statement& up::Statement::operator=(Statement&& rhs) noexcept {
    if (_stmt != rhs._stmt) {
        sqlite3_finalize(_stmt);
        _stmt = rhs._stmt;
        rhs._stmt = nullptr;
    }
    return *this;
}

void up::Statement::_begin() noexcept {
    UP_ASSERT(_stmt != nullptr);
    UP_ASSERT(!sqlite3_stmt_busy(_stmt));
    sqlite3_reset(_stmt);
}

void up::Statement::_finalize() noexcept {
    UP_ASSERT(_stmt != nullptr);
    sqlite3_reset(_stmt);
}

up::SqlResult up::Statement::_execute() noexcept {
    UP_ASSERT(!sqlite3_stmt_busy(_stmt));
    auto const rc = sqlite3_step(_stmt);
    if (rc != SQLITE_DONE) {
        return SqlResult::Error;
    }
    if (sqlite3_reset(_stmt) != SQLITE_OK) {
        return SqlResult::Error;
    }
    return SqlResult::Ok;
}

void up::Statement::_query() noexcept {
    UP_ASSERT(!sqlite3_stmt_busy(_stmt));
    sqlite3_step(_stmt);
}

bool up::Statement::_done() noexcept {
    UP_ASSERT(_stmt != nullptr);
    return sqlite3_stmt_busy(_stmt) == 0;
}

void up::Statement::_next() noexcept {
    UP_ASSERT(_stmt != nullptr);
    for (;;) {
        auto const rs = sqlite3_step(_stmt);
        if (rs == SQLITE_ROW || rs == SQLITE_DONE) {
            break;
        }
        if (rs == SQLITE_BUSY) {
            continue;
        }
        sqlite3_reset(_stmt);
        break;
    }
}

void up::Statement::_bind(int index, int64 value) noexcept {
    sqlite3_bind_int64(_stmt, index + 1, value);
}

void up::Statement::_bind(int index, string_view value) noexcept {
    sqlite3_bind_text(_stmt, index + 1, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT); // NOLINT
}

void up::Statement::_bind(int index, UUID const& value) noexcept {
    char uuidStr[UUID::strLength] = {0};
    format_to(uuidStr, "{}", value);

    sqlite3_bind_text(_stmt, index + 1, uuidStr, -1, SQLITE_TRANSIENT); // NOLINT
}

up::int64 up::Statement::_column_int64(int index) noexcept {
    return sqlite3_column_int64(_stmt, index);
}

up::zstring_view up::Statement::_column_string(int index) noexcept {
    unsigned char const* const text = sqlite3_column_text(_stmt, index);
    return text != nullptr ? zstring_view{reinterpret_cast<char const*>(text)} : ""_zsv;
}
