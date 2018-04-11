/* Copyright 2018 Andreas Baulig
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef XDKLOGGER_H_
#define XDKLOGGER_H_

#include <inttypes.h>

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"

/**
 * @brief A macro to log a message.
 *
 * @param [in] level    A log level with followed message arguments.
 */
#define LOG(level, ...) \
	do { \
		Logger_Log(level, BCDS_PACKAGE_ID, BCDS_MODULE_ID, \
				__FILE__, __LINE__, __VA_ARGS__); \
	} \
	while(false)

/**
 * @brief A macro to be used to log fatal level messages.
 */
#define LOG_FATAL(...)		LOG(LOG_LEVEL_FATAL, __VA_ARGS__)

/**
 * @brief A macro to be used to log error level messages.
 */
#define LOG_ERROR(...)		LOG(LOG_LEVEL_ERROR, __VA_ARGS__)

/**
 * @brief A macro to be used to log warning level messages.
 */
#define LOG_WARNING(...)	LOG(LOG_LEVEL_WARNING, __VA_ARGS__)

/**
 * @brief A macro to be used to log info level messages.
 */
#define LOG_INFO(...)		LOG(LOG_LEVEL_INFO, __VA_ARGS__)

/**
 * @brief A macro to be used to log debug level messages.
 */
#define LOG_DEBUG(...)		LOG(LOG_LEVEL_DEBUG, __VA_ARGS__)

/**
 * @brief Enumeration of supported log levels.
 */
typedef enum
{
	LOG_LEVEL_NONE = 0,
	LOG_LEVEL_FATAL,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	/* Add more log levels here if required */

	LOG_LEVEL_COUNT
} LogLevel_T;

/**
 * @brief Initializes the XdkLogger module.
 *
 * @return A Retcode_T noting the success of the action.
 */
Retcode_T Logger_Initialize(void);

/**
 * @brief Log a message.
 *
 * @param level
 * Log level of this message.
 * @param package
 * Package ID of the caller.
 * @param module
 * Module ID of the caller.
 * @param file
 * File from which the log message was sent (use __FILE__).
 * @param line
 * Line of call (use __LINE__).
 * @param fmt
 * Format string.
 */
void Logger_Log(LogLevel_T level, uint8_t package, uint8_t module,
		const char *file, uint32_t line, const char *fmt, ...);

/**
 * @brief Deinitializes the XdkLogger module.
 *
 * @return A Retcode_T noting the success of the action.
 */
Retcode_T Logger_Deinitialize(void);

#endif /* XDKLOGGER_H_ */
