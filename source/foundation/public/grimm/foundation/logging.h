// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "string_format.h"
#include "string_view.h"
#include "delegate.h"

namespace gm {

	/// Log severity
	enum class LogSeverity
	{
		Debug,
		Info,
		Warning,
		Error,
		Fatal,
	};

	/// Handlers that can receive logs and perform further processing on them.
	using LoggerCallback = delegate<void(string_view file, int line, LogSeverity severity, string_view msg)>;

	/// Add a new handler to the list of outputs.
	/// #FIXME: need a registration mechanism
	void GM_FRAMEWORK_API addLoggerCallback(LoggerCallback handler);

	/// Log text (with appended newline) to the most appropriate output sink for the given platform.
	/// @param file The location of the message.
	/// @param line The location of the message.
	/// @param severity The log severity.
	/// @param msg The message to display.
	void GM_FRAMEWORK_API logLine(string_view file, int line, LogSeverity severity, string_view msg);

	/// Log a cppformat formatted message (with appended newline) to the most appropriate output sink for the given platform.
	/// @tparam ParamsT Types of the parameters to be formatted.
	/// @param file The location of the message.
	/// @param line The location of the message.
	/// @param severity The log severity.
	/// @param format The format string using cppformat's syntax.
	/// @param argv The parameters to be formatted.
	template <typename... ParamsT>
	void logFormattedLine(string_view file, int line, LogSeverity severity, string_view format, ParamsT const&... argv)
	{
		format_memory_buffer buffer;
		format_into(buffer, format, argv...);
		// logLine(file, line, severity, {buffer.data(), buffer.size()});
	}

}

#define __gm_LOG_STRINGIZE_INNER(a) #a
#define __gm_LOG_STRINGIZE(token) __gm_LOG_STRINGIZE_INNER(token)

#define __gm_LOG(severity, format_string, ...) do{ ::gm::logFormattedLine(__FILE__, __LINE__, (severity), "" format_string, __VA_ARGS__); }while(false)
#define __gm_LOG_IF(pred, severity, format_string, ...) do{ if(!!(pred)) __gm_LOG((severity), "" format_string, __VA_ARGS__); }while(false)

#if !defined(NDEBUG)
#	define __gm_DEBUG_LOG(severity, format_string, ...) __gm_LOG((severity), "" format_string, __VA_ARGS__)
#	define __gm_DEBUG_LOG_IF(pred, severity, format_string, ...) do{ if(!!(pred)) __gm_LOG((severity), "" format_string, __VA_ARGS__); }while(false)
#else
#	define __gm_DEBUG_LOG(severity, format_string, ...) do{}while(false)
#	define __gm_DEBUG_LOG_IF(pred, severity, format_string, ...) do{}while(false)
#endif

#define GM_LOG_DEBUG(format_string, ...) __gm_DEBUG_LOG(::gm::LogSeverity::Debug, format_string, __VA_ARGS__)
#define GM_LOG_ERROR(format_string, ...) __gm_LOG(::gm::LogSeverity::Error, format_string, __VA_ARGS__)
#define GM_LOG_INFO(format_string, ...) __gm_LOG(::gm::LogSeverity::Info, format_string, __VA_ARGS__)

#define GM_LOG_DEBUG_IF(pred, format_string, ...) __gm_DEBUG_LOG_IF((pred), ::gm::LogSeverity::Debug, format_string, __VA_ARGS__)
#define GM_LOG_ERROR_IF(pred, format_string, ...) __gm_LOG_IF((pred), ::gm::LogSeverity::Error, format_string, __VA_ARGS__)
#define GM_LOG_INFO_IF(pred, format_string, ...) __gm_LOG_IF((pred), ::gm::LogSeverity::Info, format_string, __VA_ARGS__)
