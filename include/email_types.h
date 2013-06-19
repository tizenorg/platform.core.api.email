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


#ifndef __TIZEN_EMAIL_TYPES_H__
#define __TIZEN_EMAIL_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_MESSAGING_EMAIL_MODULE
 * @{
 */


/**
 * @file        email_types.h
 * @ingroup     CAPI_MESSAGING_FRAMEWORK
 * @brief       This file defines common types and enums of EMAIL.
 */


/**
 * @brief   Email message handle type.
 */
typedef struct email_s *email_h;


/**
 * @brief	Enumerations for the result values of email transport.
 */
typedef enum
{
  EMAIL_SENDING_FAILED = -1, /**< Email sending failed */
  EMAIL_SENDING_SUCCEEDED = 0, /**< Email sending succeeded */
} email_sending_e;


/**
 * @brief	Enumerations of the email recipient types.
 */
typedef enum
{
  EMAIL_RECIPIENT_TYPE_TO = 1, /**< Normal recipient */
  EMAIL_RECIPIENT_TYPE_CC , /**< CC(carbon copy) recipient */
  EMAIL_RECIPIENT_TYPE_BCC , /**< BCC(blind carbon copy) recipient */
} email_recipient_type_e;


/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_EMAIL_TYPES_H__*/

