// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "posql.h"

#include "potato/spud/out_ptr.h"

#include <sqlite3.h>

up::SqlResult up::Database::open(zstring_view file_name) noexcept {
    close();

    auto const rs = sqlite3_open(file_name.c_str(), out_ptr(_conn));
    if (rs != SQLITE_OK)
        return SqlResult::Error;

    return SqlResult::Ok;
}

void up::Database::close() noexcept {
    _conn.reset();
}

up::Statement up::Database::prepare(zstring_view sql) noexcept {
    SqliteStatement stmt;
    auto const rs = sqlite3_prepare_v3(_conn.get(), sql.c_str(), -1, 0, out_ptr(stmt), nullptr);
    if (rs != SQLITE_OK)
        return {};
    return Statement(std::move(stmt));
}

up::Statement::~Statement() noexcept = default;

up::SqlResult up::Statement::execute() noexcept {
    if (sqlite3_reset(_stmt.get()) != SQLITE_OK)
        return SqlResult::Error;
    if (sqlite3_step(_stmt.get()) != SQLITE_DONE)
        return SqlResult::Error;
    return SqlResult::Ok;
}
