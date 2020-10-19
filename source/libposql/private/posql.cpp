// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "posql.h"

#include "potato/runtime/assertion.h"

#include <sqlite3.h>

up::SqlResult up::Database::open(zstring_view file_name) noexcept {
    close();

    auto const rs = sqlite3_open(file_name.c_str(), &_conn);
    if (rs != SQLITE_OK)
        return SqlResult::Error;

    return SqlResult::Ok;
}

void up::Database::close() noexcept {
    sqlite3_close(_conn);
    _conn = nullptr;
}

up::Statement up::Database::prepare(zstring_view sql) noexcept {
    sqlite3_stmt* stmt = nullptr;
    auto const rs = sqlite3_prepare_v3(_conn, sql.c_str(), -1, 0, &stmt, nullptr);
    if (rs != SQLITE_OK)
        return {};
    return Statement(stmt);
}

up::SqlResult up::Database::execute(zstring_view sql) noexcept {
    auto const rs = sqlite3_exec(_conn, sql.c_str(), nullptr, nullptr, nullptr);
    if (rs != SQLITE_OK)
        return SqlResult::Error;
    return SqlResult::Ok;
}

up::Statement::~Statement() noexcept {
    sqlite3_finalize(_stmt);
}

void up::Statement::_begin() noexcept {
    UP_ASSERT(!sqlite3_stmt_busy(_stmt));
    sqlite3_reset(_stmt);
}

void up::Statement::_finalize() noexcept {
    sqlite3_reset(_stmt);
}

up::SqlResult up::Statement::_execute() noexcept {
    UP_ASSERT(!sqlite3_stmt_busy(_stmt));
    if (sqlite3_step(_stmt) != SQLITE_DONE)
        return SqlResult::Error;
    if (sqlite3_reset(_stmt) != SQLITE_OK)
        return SqlResult::Error;
    return SqlResult::Ok;
}

void up::Statement::_query() noexcept {
    UP_ASSERT(!sqlite3_stmt_busy(_stmt));
    sqlite3_step(_stmt);
}

bool up::Statement::_done() noexcept {
    return sqlite3_stmt_busy(_stmt) == 0;
}

void up::Statement::_next() noexcept {
    for (;;) {
        auto const rs = sqlite3_step(_stmt);
        if (rs == SQLITE_ROW || rs == SQLITE_DONE) {
            break;
        }
        else if (rs == SQLITE_BUSY) {
            continue;
        }
        else {
            sqlite3_reset(_stmt);
            break;
        }
    }
}

void up::Statement::_bind(int index, int64 value) noexcept {
    sqlite3_bind_int64(_stmt, index + 1, value);
}

void up::Statement::_bind(int index, zstring_view value) noexcept {
    sqlite3_bind_text(_stmt, index + 1, value.c_str(), -1, nullptr);
}

up::int64 up::Statement::_column_int64(int index) noexcept {
    return sqlite3_column_int64(_stmt, index);
}

up::zstring_view up::Statement::_column_string(int index) noexcept {
    return reinterpret_cast<char const*>(sqlite3_column_text(_stmt, index));
}
