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

#include <email-types.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_ATTACHEMENT_COUNT           50

#define MAX_RECIPIENT_ADDRESS_LEN 	234

typedef struct _email_s {
	email_mailbox_t *mbox;
	email_mail_data_t *mail;
	email_attachment_data_t attachment[MAX_ATTACHEMENT_COUNT];
} email_s;

typedef  struct _mailstatus_s {
	int mailId;
	int accountId;
	int status;
	int errorCode;
} mailstatus_s;

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_EMAIL_PRIVATE_TYPES_H__*/
