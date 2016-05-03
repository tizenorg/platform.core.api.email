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

 #ifndef __MESSAGING_EMAIL_H__
 #define __MESSAGING_EMAIL_H__

/**
 * @addtogroup CAPI_MESSAGING_EMAIL_MODULE
 * @{
 */

/**
 * @file        email.h
 * @ingroup     CAPI_MESSAGING_EMAIL_MODULE
 * @brief       Messaging API file, support for sending email messages.
 */


#include <stdlib.h>
#include <stdio.h>
#include <email_types.h>
#include <email_error.h>
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief   Creates an email message handle for sending an email message.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/email
 *
 * @remarks  You must release @a email using email_destroy_message().
 *
 * @param[out]  email  A handle to the email message
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE                  Successful
 * @retval  #EMAILS_ERROR_OUT_OF_MEMORY         Out of memory
 * @retval  #EMAILS_ERROR_ACCOUNT_NOT_FOUND     Email account not found
 * @retval  #EMAILS_ERROR_PERMISSION_DENIED     The application does not have the privilege to call this method
 *
 * @pre      At least one email account should be set up on the device.
 *
 * @see email_destroy_message()
 */
int email_create_message(email_h *email);

/**
 * @brief   Destroys the email message handle and releases all its resources.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]  email  The handle to the email message
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE               Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval  #EMAILS_ERROR_OPERATION_FAILED   Operation failed
 *
 * @see email_create_message()
 */
int email_destroy_message(email_h email);

/**
 * @brief   Sets a subject of the email message.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/email

 * @param[in]   email    The handle to the email message
 * @param[in]   subject  The subject of the email message
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE               Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval  #EMAILS_ERROR_OUT_OF_MEMORY      Out of memory
 * @retval  #EMAILS_ERROR_PERMISSION_DENIED  The application does not have the privilege to call this method
 *
 * @pre     An email message handle is created using email_create_message().
 *
 * @see  email_create_message()
 */
int email_set_subject(email_h email, const char *subject);

/**
 * @brief   Populates a body of the email message.
 * @details Email message body means the text data to be delivered.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediastorage
 *
 * @param[in]   email   The handle to the email message
 * @param[in]   body    The message body
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE              Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval  #EMAILS_ERROR_OPERATION_FAILED  Operation failed
 * @retval  #EMAILS_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 *
 * @pre     An email message handle is created using email_create_message().
 *
 * @see  email_create_message()
 */
int email_set_body(email_h email, const char *body);

/**
 * @brief   Adds a recipient to the email message.
 * @details The email API supports sending an email message to multiple recipients.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @remarks Email address should be in standard format (as described in
 *          Internet standards RFC 5321 and RFC 5322).
 *
 * @param[in]   email   The handle to the email message
 * @param[in]   type    The recipient type
 * @param[in]   address The recipient email address
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE              Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval  #EMAILS_ERROR_OUT_OF_MEMORY     Out of memory
 *
 * @pre     An email message handle is created using email_create_message().
 *
 * @see email_create_message()
 * @see email_remove_all_recipients()
 */
int email_add_recipient(email_h email, email_recipient_type_e type, const char *address);

/**
 * @brief   Removes all recipients for the email message.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]  email  The handle to the email message
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE              Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @pre     An email message handle is created using email_create_message().
 *
 * @see email_add_recipient()
 */
int email_remove_all_recipients(email_h email);

/**
 * @brief   Adds a file as an attachment to the email message.
 * @details It should be used to add a file to the attachment list
 *          of the email message.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @remarks  The maximum attachment file size is 10MB.
 *           http://tizen.org/privilege/mediastorage is needed if input or output path are relevant to media storage
 *           http://tizen.org/privilege/externalstorage is needed if input or output path are relevant to external storage.
 *
 * @param[in]   email       The handle to the email message
 * @param[in]   filepath    The absolute full path of the file to be attached
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE              Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval  #EMAILS_ERROR_OUT_OF_MEMORY     Out of memory
 * @retval  #EMAILS_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 *
 * @pre     An email message handle is created using email_create_message().
 *
 * @see email_remove_all_attachments()
 *
 */
int email_add_attach(email_h email, const char *filepath);

/**
 * @brief   Clears all attachments of the email message.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]  email  The handle to the email message
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE              Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @pre     An email message handle is created using email_create_message().
 *
 * @see email_create_message()
 * @see email_add_attach()
 */
int email_remove_all_attachments(email_h email);

/**
 * @brief   Saves the email message at outbox.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/email
 *
 * @param[in]  email  The handle to the email message
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE                             Successful
 * @retval  #EMAILS_ERROR_COMMUNICATION_WITH_SERVER_FAILED Communication with server failed.
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER                Invalid parameter
 * @retval  #EMAILS_ERROR_PERMISSION_DENIED 		   The application does not have the privilege to call this method
 *
 * @pre     An email message handle is created using email_create_message().
 *
 * @see email_create_message()
 * @see email_add_recipient()
 * @see email_set_body()
 * @see email_save_message
 */
int email_save_message(email_h email);


/**
 * @brief   Sends the email message.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 *
 * @remarks In order to check whether sending a message succeeds,
 *          you should register email_message_sent_cb() using email_set_message_sent_cb().
 *
 * @param[in]   email            The handle to the email message
 * @param[in]   save_to_sentbox  Set to @c true to save the message in the sentbox,
 *                               otherwise set to @c false to not save the message in the sentbox
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE                              Successful
 * @retval  #EMAILS_ERROR_COMMUNICATION_WITH_SERVER_FAILED  Communication with server failed
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER                 Invalid parameter
 *
 * @pre     An email message is stored using email_save_message().
 *
 * @see email_save_message()
 * @see email_set_message_sent_cb()
 */
int email_send_message(email_h email, bool save_to_sentbox);


/**
 * @brief   Called when the process of sending an email finishes.
 * @details You can check whether sending an email succeeds using this function.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   email       The handle to the email message
 * @param[in]   result      The result of email message sending \n
 *                          #EMAIL_SENDING_FAILED or #EMAIL_SENDING_SUCCEEDED
 * @param[in]   user_data   The user data passed from the callback registration function
 *
 * @pre email_send_message() will invoke this callback if you register this callback using email_set_message_sent_cb().
 *
 * @see email_send_message()
 * @see email_set_message_sent_cb()
 * @see email_unset_message_sent_cb()
 */
typedef void (*email_message_sent_cb)(email_h email, email_sending_e result, void *user_data);

/**
 * @brief   Registers a callback function to be invoked when an email message is sent.
 * @details You will be notified when sending a message finishes and check whether it succeeds using this function.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]  email      The handle to the email message
 * @param[in]  callback   The callback function to register
 * @param[in]  user_data  The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE               Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post  It will invoke email_message_sent_cb().
 *
 * @see email_message_sent_cb()
 * @see email_unset_message_sent_cb()
 * @see email_send_message()
 */
int email_set_message_sent_cb(email_h email, email_message_sent_cb callback, void *user_data);

/**
 * @brief   Unregisters the callback function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]  msg  The handle to the email message
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #EMAILS_ERROR_NONE              Successful
 * @retval  #EMAILS_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see email_message_sent_cb()
 * @see email_set_message_sent_cb()
 * @see email_send_message()
 */
int email_unset_message_sent_cb(email_h msg);

#ifdef __cplusplus
}
#endif

/**
* @}
*/

#endif /* __MESSAGING_EMAIL_H__ */

