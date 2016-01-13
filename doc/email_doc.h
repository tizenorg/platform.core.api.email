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


#ifndef __TIZEN_EMAIL_DOC_H__
#define __TIZEN_EMAIL_DOC_H__

/**
 * @defgroup CAPI_MESSAGING_EMAIL_MODULE Email
 * @brief The Email API provides functions to create, set properties and send email.
 * @ingroup CAPI_MESSAGING_FRAMEWORK
 *
 * @addtogroup CAPI_MESSAGING_EMAIL_MODULE
 *
 * @section CAPI_MESSAGING_EMAIL_MODULE_HEADER Required Header
 *   \#include <email.h>
 *
 *
 * @section CAPI_MESSAGING_EMAIL_MODULE_OVERVIEW Overview
 *
 * The @ref CAPI_MESSAGING_EMAIL_MODULE API provides functions that prepare and send email messages. 
 * This API allows email message creation, setting message properties 
 * and sending as well setting up to be notified when the email message has been sent.
 *
 *
 * Email, short for electronic mail, is a method of exchanging digital messages.
 * This API allows you to send email using SMTP. 
 * Simple Mail Transfer Protocol (SMTP) used for sending email via Internet is described in RFC5321/5322 standards.
 *
 * The Email API consists of functions that:
 * - Set the subject, body and recipients of an email message
 * - Set the file path for attaching files to an email message
 * - Send an email message
 * - Register/unregister a callback function to be called when the sending process is complete,
 *     whether it is sent successfully or not
 *
 * Email sending is asynchronous and the application should not wait for the result.  
 * Not only may the process be slow (connections to be established and so on), 
 * but even if the mail server is not available a message send may not be a failure, if there is a spooling mechanism.
 * Instead, the callback function is used to receive status. In addition, note that once email_send_message() is called, 
 * the message contents are out of the application's hands.
 * Even if the message appears not to have finished sending, it can no longer be modified.
 *
 * @subsection CAPI_MESSAGING_EMAIL_CALLBACK_OPERATIONS Callback(Event) Operations
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b>REGISTER</b></th>
 *        <th><b>UNREGISTER</b></th>
 *        <th><b>CALLBACK</b></th>
 *        <th><b>DESCRIPTION</b></th>
 *     </tr>
 *     <tr>
 *        <td>email_set_message_sent_cb()</td>
 *        <td>email_unset_message_sent_cb()</td>
 *        <td>email_message_sent_cb()</td>
 *        <td>Registering/unregistering a callback function to check whether an email message is sent successfully or not </td>
 *     </tr>
 *</table></div>
 * <BR>
 */


#endif /* __TIZEN_EMAIL_DOC_H__ */
