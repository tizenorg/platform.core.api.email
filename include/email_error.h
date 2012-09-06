/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
 *  @brief    Enumerations of error codes for email API.
 */
typedef enum
{
	EMAIL_ERROR_NONE                              = TIZEN_ERROR_NONE,                   /**< Successful */
	EMAIL_ERROR_OUT_OF_MEMORY                     = TIZEN_ERROR_OUT_OF_MEMORY,          /**< Memory cannot be allocated */
	EMAIL_ERROR_INVALID_PARAMETER                 = TIZEN_ERROR_INVALID_PARAMETER,      /**< Invalid parameter */

	EMAIL_ERROR_SERVER_NOT_READY                  = TIZEN_ERROR_MESSAGING_CLASS|0x200,  /**< Server not ready */
	EMAIL_ERROR_COMMUNICATION_WITH_SERVER_FAILED  = TIZEN_ERROR_MESSAGING_CLASS|0x201,  /**< Communication with server failed */
	EMAIL_ERROR_OPERATION_FAILED                  = TIZEN_ERROR_MESSAGING_CLASS|0x202,  /**< Operation failed */
	EMAIL_ERROR_ACCOUNT_NOT_FOUND                 = TIZEN_ERROR_MESSAGING_CLASS|0x203,  /**< Email account not found */
	EMAIL_ERROR_DB_FAILED                         = TIZEN_ERROR_MESSAGING_CLASS|0x204,  /**< Email database failed */
} email_error_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EMAILAPI_ERROR_H__*/
