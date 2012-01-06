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


#ifndef __TIZEN_EMAIL_PRIVATE_TYPES_H__
#define __TIZEN_EMAIL_PRIVATE_TYPES_H__

#include <emf-types.h>
#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_RECIPIENT_ADDRESS_LEN 	234
#define MAX_RECIPIENT_COUNT		50 
#define MAX_RECIPIENT_ADDRESSES_LEN	(MAX_RECIPIENT_ADDRESS_LEN*MAX_RECIPIENT_COUNT)

typedef struct  _email_s{
	
	emf_mail_t *mail;
	emf_mailbox_t *mbox;
} email_s;

typedef  struct _mailstatus_s {
        int mailId;
        int accountId;
        int status;
        int errorCode;
    }mailstatus_s;

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_EMAIL_PRIVATE_TYPES_H__*/
