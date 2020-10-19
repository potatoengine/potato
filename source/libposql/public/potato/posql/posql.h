// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/typelist.h"
#include "potato/spud/zstring_view.h"

#include <tuple>

extern "C" {
struct sqlite3;
struct sqlite3_stmt;
}

namespace up {
    inline namespace posql {
        enum class SqlResult { Ok = 0, Error };

        class Database;
        class Statement;

        template <typename...>
        class QueryResult;
        struct QueryResultSentinel {};
        template <typename...>
        class QueryResultIterator;

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

        template <typename... T>
        class QueryResult {
        public:
            inline ~QueryResult() noexcept;

            using value_type = std::tuple<T...>;
            using iterator = QueryResultIterator<T...>;
            using sentinel = QueryResultSentinel;

            iterator begin() noexcept { return iterator(_stmt); }
            sentinel end() noexcept { return sentinel{}; }

        private:
            explicit QueryResult(Statement& stmt) : _stmt(stmt) {}

            Statement& _stmt;

            friend Statement;
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

            UP_POSQL_API [[nodiscard]] SqlResult execute() noexcept {
                _begin();
                return _execute();
            }

            template <typename... T>
            [[nodiscard]] SqlResult execute(T const&... args) noexcept {
                _begin();
                _bind(std::make_integer_sequence<int, sizeof...(args)>{}, args...);
                return _execute();
            }

            template <typename... R>
            [[nodiscard]] QueryResult<R...> query() noexcept {
                _begin();
                _query();
                return QueryResult<R...>{*this};
            }

            template <typename... R, typename... T>
            [[nodiscard]] QueryResult<R...> query(T const&... args) noexcept {
                _begin();
                _bind(std::make_integer_sequence<int, sizeof...(args)>{}, args...);
                _query();
                return QueryResult<R...>{*this};
            }

        private:
            UP_POSQL_API void _begin() noexcept;
            UP_POSQL_API SqlResult _execute() noexcept;
            UP_POSQL_API void _query() noexcept;
            UP_POSQL_API bool _done() noexcept;
            UP_POSQL_API void _next() noexcept;
            UP_POSQL_API void _finalize() noexcept;

            template <int... I, typename... T>
            void _bind(std::integer_sequence<int, I...>, T const&... args) noexcept {
                (void)(_bind(I, args), ...);
            }

            UP_POSQL_API void _bind(int index, int64 value) noexcept;
            UP_POSQL_API void _bind(int index, zstring_view value) noexcept;

            template <typename... T>
            std::tuple<T...> _columns() {
                return _columns(std::make_integer_sequence<int, sizeof...(T)>{}, typelist<T...>{});
            }

            template <int... I, typename... T>
            std::tuple<T...> _columns(std::integer_sequence<int, I...>, typelist<T...>) {
                return std::tuple<T...>(_column<T>(I)...);
            }

            template <typename T>
            auto _column(int index) {
                if constexpr (std::is_constructible_v<T, char const*>) {
                    return _column_string(index);
                }
                else if constexpr (std::is_constructible_v<T, int64>) {
                    return _column_int64(index);
                }
                else if constexpr (std::is_enum_v<T>) {
                    return T(_column_int64(index));
                }
            }

            UP_POSQL_API [[nodiscard]] int64 _column_int64(int index) noexcept;
            UP_POSQL_API [[nodiscard]] zstring_view _column_string(int index) noexcept;

            template <typename... T>
            friend class QueryResult;
            template <typename... T>
            friend class QueryResultIterator;

            sqlite3_stmt* _stmt = nullptr;
        };

        template <typename... T>
        class QueryResultIterator {
        public:
            [[nodiscard]] bool operator==(QueryResultSentinel) noexcept { return _stmt._done(); }
            QueryResultIterator& operator++() noexcept {
                _stmt._next();
                return *this;
            }
            inline [[nodiscard]] std::tuple<T...> operator*() { return _stmt._columns<T...>(); }

        private:
            explicit QueryResultIterator(Statement& stmt) : _stmt(stmt) {}

            Statement& _stmt;

            template <typename... T>
            friend class QueryResult;
        };

        template <typename... T>
        QueryResult<T...>::~QueryResult() noexcept {
            _stmt._finalize();
        }
    }; // namespace posql

} // namespace up
