/*
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __EMAILAPI_ERROR_H__
#define __EMAILAPI_ERROR_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_MESSAGING_EMAIL_MODULE
 * @{
 */

/**
 * @file        email_error.h
 * @brief       Email error definitions.
 */

/**
 *  @brief    Enumeration for error codes for email API.
 *  @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 */
typedef enum {
	EMAILS_ERROR_NONE                              = TIZEN_ERROR_NONE,                   /**< Successful */
	EMAILS_ERROR_OUT_OF_MEMORY                     = TIZEN_ERROR_OUT_OF_MEMORY,          /**< Memory cannot be allocated */
	EMAILS_ERROR_INVALID_PARAMETER                 = TIZEN_ERROR_INVALID_PARAMETER,      /**< Invalid parameter */

	EMAILS_ERROR_SERVER_NOT_READY                  = TIZEN_ERROR_EMAIL_SERVICE|0x200,    /**< Server not ready */
	EMAILS_ERROR_COMMUNICATION_WITH_SERVER_FAILED  = TIZEN_ERROR_EMAIL_SERVICE|0x201,    /**< Communication with server failed */
	EMAILS_ERROR_OPERATION_FAILED                  = TIZEN_ERROR_EMAIL_SERVICE|0x202,    /**< Operation failed */
	EMAILS_ERROR_ACCOUNT_NOT_FOUND                 = TIZEN_ERROR_EMAIL_SERVICE|0x203,    /**< Email account not found */
	EMAILS_ERROR_DB_FAILED                         = TIZEN_ERROR_EMAIL_SERVICE|0x204,    /**< Email database failed */
	EMAILS_ERROR_PERMISSION_DENIED                 = TIZEN_ERROR_PERMISSION_DENIED,      /**< Permission denied */
} email_error_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EMAILAPI_ERROR_H__*/

