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
#include "XdkApp.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID	APP_MODULE_LOGGER

#include "XdkLogger.h"

#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"

#define LOGGER_LOG_BUFFER_SIZE	(UINT32_C(1024))

static char LogBuffer[LOGGER_LOG_BUFFER_SIZE];

Retcode_T Logger_Initialize(void)
{
	Retcode_T rc = RETCODE_OK;

	return rc;
}

void Logger_Log(LogLevel_T level, uint8_t package, uint8_t module,
		const char *file, uint32_t line, const char *fmt, ...)
{
	BCDS_UNUSED(package);
	BCDS_UNUSED(module);

	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(LogBuffer, sizeof(LogBuffer), fmt, args);
	va_end(args);

	switch (level)
	{
	case LOG_LEVEL_NONE:
		printf("%.*s", len, LogBuffer);
		break;
	case LOG_LEVEL_FATAL:
		printf("FATAL: %.*s -> %s:%"PRIu32"\n", len, LogBuffer, file, line);
		break;
	case LOG_LEVEL_ERROR:
		printf("ERROR: %.*s -> %s:%"PRIu32"\n", len, LogBuffer, file, line);
		break;
	case LOG_LEVEL_WARNING:
		printf("WARNING: %.*s\n", len, LogBuffer);
		break;
	case LOG_LEVEL_INFO:
		printf("INFO: %.*s\n", len, LogBuffer);
		break;
	case LOG_LEVEL_DEBUG:
		printf("DEBUG: %.*s\n", len, LogBuffer);
		break;
	default:
		break;
	}
}

Retcode_T Logger_Deinitialize(void)
{
	Retcode_T rc = RETCODE_OK;

	return rc;
}
