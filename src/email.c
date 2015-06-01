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


#include <memory.h>
#include <dlog.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <email-api.h>

#include <email.h>
#include <email_private.h>
#include <email_types.h>
#include <email_error.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CAPI_EMAIL"

#define EM_SAFE_STRDUP(s) \
({\
	char* _s = (char*)s;\
	(_s)? strdup(_s) : NULL;\
})

#define EM_SAFE_STRLEN(s) \
({\
	char* _s = (char*)s;\
	(_s)? strlen(_s) : 0;\
})

#define FILE_PATH "/opt/usr/media/.email/capimail.txt"
#define TMP_PATH "/opt/usr/media/.email"

static guint g_dbus_return_id = 0;

typedef struct {
	email_message_sent_cb  callback;
	email_s *handle;
	void *user_data;
} email_cb_context;

GSList *gEmailcbList= NULL;

GDBusConnection* connection=NULL;
//------------- Utility Or Miscellaneous
void _email_add_dbus_filter(void);
void _email_remove_dbus_filter(void);
int _email_error_converter(int err, const char *func, int line);
int _email_copy_handle(email_s **dst_handle, email_s *src_handle);
void _email_free_cb_context(email_cb_context *cbcontext);

#define CONVERT_ERROR(err) _email_error_converter(err, __FUNCTION__, __LINE__);

//------------------------------------

int email_create_message(email_h *msg)
{
	LOGD("START\n");
	int ret;
	email_s * msg_s = NULL;
	email_account_t* account = NULL;
	int len;

	if(msg == NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}

	// 1. create service for ipc
	ret = email_service_begin();
	if (ret != EMAIL_ERROR_NONE && ret != EMAIL_ERROR_IPC_ALREADY_INITIALIZED) {
		SECURE_SLOGE("[%s] email_service_begin failed : [%d]",__FUNCTION__, ret);
		return CONVERT_ERROR(ret);
	}

	msg_s= (email_s*)calloc(1,sizeof(email_s));
	if (msg_s != NULL)
	{
		msg_s->mail = (email_mail_data_t *)calloc(1,sizeof(email_mail_data_t));
		if (msg_s->mail == NULL) {
			SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
			free(msg_s);
			return EMAILS_ERROR_OUT_OF_MEMORY;
		}

		msg_s->mbox = (email_mailbox_t *)calloc(1,sizeof(email_mailbox_t));
		if (msg_s->mbox == NULL)
		{
			SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mbox", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
			email_free_mail_data(&msg_s->mail, 1);
			free(msg_s);
			return EMAILS_ERROR_OUT_OF_MEMORY;
		}
	}
	else
	{
		SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
		return EMAILS_ERROR_OUT_OF_MEMORY;
	}

		
	//return error from F/W 
	//EMAILS_ERROR_INVALID_PARAM/EMAIL_ERROR_NONE/EMAILS_ERROR_DB_FAILURE/EMAILS_ERROR_ACCOUNT_NOT_FOUND/EMAILS_ERROR_OUT_OF_MEMORY
	int default_account_id = 0;
	if ((ret = email_load_default_account_id(&default_account_id)) != EMAIL_ERROR_NONE) {
		SECURE_SLOGE("[%s] email_load_default_account_id failed : [%d]",__FUNCTION__, ret);
		email_free_mail_data(&msg_s->mail, 1);
		email_free_mailbox(&msg_s->mbox, 1);
		free(msg_s);
		return CONVERT_ERROR(ret);
	}
	
	ret = email_get_account(default_account_id, GET_FULL_DATA_WITHOUT_PASSWORD, &account);
	if (ret!=EMAIL_ERROR_NONE) {
		SECURE_SLOGE("[%s] email_get_account failed : [%d]",__FUNCTION__, ret);
		if (account) {
			email_free_account(&account, 1);
		}
		email_free_mail_data(&msg_s->mail, 1);
		email_free_mailbox(&msg_s->mbox, 1);
		free(msg_s);
		return CONVERT_ERROR(ret);
	}

	SECURE_LOGD("account address = %s",account->user_email_address);
	SECURE_LOGD("account id = %d",account->account_id);
	SECURE_LOGD("account name = %s",account->account_name);
	SECURE_LOGD("account user_name = %s",account->incoming_server_user_name);
	
	len= EM_SAFE_STRLEN(account->incoming_server_user_name)+EM_SAFE_STRLEN(account->user_email_address)+1+1+1+1+1;
	msg_s->mail->full_address_from = (char *)calloc(1,sizeof(char)*(len));//"++"+<+ address +> + NULL
	char *strfrom = msg_s->mail->full_address_from;

	if (account->incoming_server_user_name)
		snprintf(strfrom, len, "%s%s%s%s%s%s", "\"", account->incoming_server_user_name, "\"", "<", account->user_email_address, ">");
	else
		snprintf(strfrom, len, "%s%s%s", "<", account->user_email_address, ">");

	//mbox
	email_mailbox_t *mbox = msg_s->mbox;

	if ((ret = email_get_mailbox_by_mailbox_type(default_account_id, EMAIL_MAILBOX_TYPE_OUTBOX, &mbox)) != EMAIL_ERROR_NONE) {
		SECURE_SLOGE("[%s] email_get_mailbox_by_mailbox_type failed %d", __FUNCTION__, ret);
		email_free_mail_data(&msg_s->mail, 1);
		email_free_mailbox(&msg_s->mbox, 1);
		free(msg_s);
		return CONVERT_ERROR(ret);
	}

	//info
	msg_s->mail->account_id = account->account_id;
	msg_s->mail->flags_draft_field = 1;
	msg_s->mail->flags_seen_field = 1;
	msg_s->mail->priority = EMAIL_MAIL_PRIORITY_NORMAL;
	msg_s->mail->mailbox_id = mbox->mailbox_id;
	msg_s->mail->mailbox_type = mbox->mailbox_type;
	msg_s->mail->attachment_count = 0;

	*msg = (email_h)msg_s;
	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int email_destroy_message(email_h msg)
{
	LOGD("START\n");
	int ret;


	if(msg == NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}
		
	email_s* msg_s = (email_s* )msg;

	if(msg_s)
	{
		if (msg_s->mail)
			email_free_mail_data(&msg_s->mail, 1);

		if (msg_s->mbox)
			email_free_mailbox(&msg_s->mbox, 1);

		free(msg_s);		
	}
	
	ret=email_service_end();
	
	if(ret!=EMAIL_ERROR_NONE){
		SECURE_SLOGE("[%s] OPERATION_FAILED(0x%08x) : Finishing email service failed", __FUNCTION__, EMAILS_ERROR_OPERATION_FAILED);
		return EMAILS_ERROR_OPERATION_FAILED;
	}

	gEmailcbList= NULL;

	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int email_set_subject (email_h msg, const char *subject)
{
	LOGD("START\n");
	int len;
	if(msg ==NULL)
	{
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}
		
	email_s* msg_s = (email_s* )msg;	

	msg_s->mail->subject=(char*)calloc(1,sizeof(char)*strlen(subject)+1);

	if(msg_s->mail->subject ==NULL)
	{
		SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail->head->subject", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
		return EMAILS_ERROR_OUT_OF_MEMORY;
	}
	
	len =strlen(subject)+1;
	snprintf(msg_s->mail->subject ,len,"%s",subject); 
	
	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int email_set_body (email_h msg, const char *body)
{
	LOGD("START\n");
	int len;

	if (msg == NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}
	
	email_s* msg_s = (email_s* )msg;	
	FILE* file = NULL;

	if (!g_file_test(TMP_PATH, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		if (g_mkdir(TMP_PATH, 0775) == -1) {
			SECURE_SLOGE("[%s] OPERATION_FAILED(0x%08x) : Create directory failed.", __FUNCTION__, EMAILS_ERROR_OPERATION_FAILED);
			return EMAILS_ERROR_OPERATION_FAILED;
		}

		if (g_chmod(TMP_PATH, 0775) == -1) {
			SECURE_SLOGE("[%s] OPERATION_FAILED(0x%08x) : Change permission failed.", __FUNCTION__, EMAILS_ERROR_OPERATION_FAILED);
			return EMAILS_ERROR_OPERATION_FAILED;
		}
	}

	file = fopen(FILE_PATH, "w");
	if (file != NULL) {
		 fputs(body, file);
		 fclose(file);
	} else {
		SECURE_SLOGE("[%s] OPERATION_FAILED(0x%08x) : opening file for email body failed.", __FUNCTION__, EMAILS_ERROR_OPERATION_FAILED);
		return EMAILS_ERROR_OPERATION_FAILED;
	}

	len = strlen(FILE_PATH) + 1;

	msg_s->mail->file_path_plain = (char *)calloc(1, sizeof(char) * len);
	if (msg_s->mail->file_path_plain == NULL) {
		SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to allocate body(plain).", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
		return EMAILS_ERROR_OUT_OF_MEMORY;
	}
	
	snprintf(msg_s->mail->file_path_plain, len, "%s", FILE_PATH);
	
	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int email_add_recipient (email_h msg, email_recipient_type_e type, const char *address)
{
	LOGD("START\n");
	char *tmp;
	int total_len,in_len,pre_len,len;

	if(msg == NULL || type < EMAIL_RECIPIENT_TYPE_TO || type > EMAIL_RECIPIENT_TYPE_BCC )
	{
		return EMAILS_ERROR_INVALID_PARAMETER;
	}
	
	email_s* msg_s = (email_s* )msg;
	
	if(strlen(address) > MAX_RECIPIENT_ADDRESS_LEN)
	{
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : The length of address should be less than 234.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}

	if(type == EMAIL_RECIPIENT_TYPE_TO)
	{
		if(msg_s->mail->full_address_to == NULL)
		{
			msg_s->mail->full_address_to = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(msg_s->mail->full_address_to == NULL)
			{
				SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->to.", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
			 	return EMAILS_ERROR_OUT_OF_MEMORY;
			}
			len =strlen(address)+2+1+1;
			snprintf(msg_s->mail->full_address_to,len,"%s%s%s","<",address,">");
		} else {
			in_len = strlen(address);
			pre_len = strlen(msg_s->mail->full_address_to);
			total_len = pre_len+in_len+3+1;// length of ",<>" + NULL

			//add new address
			tmp = msg_s->mail->full_address_to;
			msg_s->mail->full_address_to = (char*)calloc(1,sizeof(char)*total_len);
			snprintf(msg_s->mail->full_address_to,total_len,"%s%s%s%s",tmp,",<",address,">");
			free(tmp);
		}
		
	}
	else if(type == EMAIL_RECIPIENT_TYPE_CC)//MESSAGING_RECIPIENT_TYPE_CC
	{
		if(msg_s->mail->full_address_cc == NULL)
		{
			msg_s->mail->full_address_cc = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(msg_s->mail->full_address_cc == NULL)
			{
				SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->cc.", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
				return EMAILS_ERROR_OUT_OF_MEMORY;
			}
			len =strlen(address)+2+1+1;
			snprintf(msg_s->mail->full_address_cc,len,"%s%s%s","<",address,">");
		} else {

			in_len = strlen(address);
			pre_len = strlen(msg_s->mail->full_address_cc);
			total_len = pre_len+in_len+3+1;// length of ",<>" + NULL

			//add new address
			tmp = msg_s->mail->full_address_cc;
			msg_s->mail->full_address_cc = (char*)calloc(1,sizeof(char)*total_len);
			snprintf(msg_s->mail->full_address_cc,total_len,"%s%s%s%s",tmp,",<",address,">");
			free(tmp);
		}
		
	}
	else //MESSAGING_RECIPIENT_TYPE_BCC
	{
		if(msg_s->mail->full_address_bcc == NULL)
		{
			msg_s->mail->full_address_bcc = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(msg_s->mail->full_address_bcc==NULL)
			{
				SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->bcc.", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
				return EMAILS_ERROR_OUT_OF_MEMORY;
			}
			len =strlen(address)+2+1+1;
			snprintf(msg_s->mail->full_address_bcc,len,"%s%s%s","<",address,">");
		} else {

			in_len = strlen(address);
			pre_len = strlen(msg_s->mail->full_address_bcc);
			total_len = pre_len+in_len+3+1;// length of ",<>" + NULL

			//add new address
			tmp = msg_s->mail->full_address_bcc;
			msg_s->mail->full_address_bcc = (char*)calloc(1,sizeof(char)*total_len);
			snprintf(msg_s->mail->full_address_bcc,total_len,"%s%s%s%s",tmp,",<",address,">");
			free(tmp);
		}
	}

	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int email_remove_all_recipients(email_h msg)
{
	LOGD("START\n");
	if(msg ==NULL)
	{
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : fail to create tmp memory.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}
	
	email_s *msg_s = (email_s *)msg;

	if(msg_s->mail->full_address_to != NULL)
	{
		free(msg_s->mail->full_address_to);
		msg_s->mail->full_address_to = NULL;
	}

	if(msg_s->mail->full_address_cc != NULL)
	{
		free(msg_s->mail->full_address_cc);
		msg_s->mail->full_address_cc = NULL;
	}

	if(msg_s->mail->full_address_bcc != NULL)
	{
		free(msg_s->mail->full_address_bcc);
		msg_s->mail->full_address_bcc = NULL;
	}
	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}


int email_add_attach (email_h msg, const char *filepath)
{
	LOGD("START\n");
	int len;
	char *pos,*last;
	struct stat st;

	if(msg ==NULL || filepath == NULL)
	{
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg or filepath is null.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}

	email_s *msg_s = (email_s *)msg;

	int attachment_count = msg_s->mail->attachment_count;
	email_attachment_data_t *new_attach = msg_s->attachment;

	stat(filepath, &st);
	if(st.st_size > 10*1024*1024)
	{
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : the size of attachment file is beyond the limit(MAX:10M).", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}

	if (!S_ISREG(st.st_mode))
	{
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : the filepath is not regular file.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}
		

	pos=strpbrk(filepath, "//");
	len =strlen(filepath); 
	if(pos == NULL)
	{
		new_attach[attachment_count].attachment_name = (char*)calloc(1,sizeof(char)*len+1);
		snprintf(new_attach[attachment_count].attachment_name,len+1,"%s",filepath); 
	}
	else
	{
		while(pos != NULL)
		{
			last=pos;
			pos=strpbrk(pos,"//");	
			if(pos != NULL) pos++;
		}

		new_attach[attachment_count].attachment_name = strdup(last);
	}
	new_attach[attachment_count].attachment_path =(char*)calloc(1,sizeof(char)*len+1);
	if (new_attach[attachment_count].attachment_path == NULL) return EMAILS_ERROR_OUT_OF_MEMORY;

	snprintf(new_attach[attachment_count].attachment_path, len+1, "%s", filepath); 
	new_attach[attachment_count].attachment_size = st.st_size;
	new_attach[attachment_count].save_status = 1;

	msg_s->mail->attachment_count++;

	LOGD("END\n");
	return EMAILS_ERROR_NONE;

}

int email_remove_all_attachments (email_h msg)
{
	LOGD("START\n");
	if (msg ==NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}

	email_s* msg_s = (email_s* )msg;

	msg_s->mail->attachment_count = 0;

	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int email_save_message (email_h msg)
{
	LOGD("START\n");
	int ret;
/*
	int i;
	email_mailbox_t *box = NULL;
	email_attachment_data_t *tmp_attach = NULL;
	struct tm *struct_time;
*/
	if (msg == NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}	

	email_s *msg_s = (email_s *)msg;
#if 0	
	/*--------- head ----------*/
	SECURE_LOGD(" ----------head---------");
	SECURE_LOGD("  mid: %s\n",msg_s->mail->message_id);
	SECURE_LOGD("  subject: %s\n",msg_s->mail->subject);
	SECURE_LOGD("  to: %s\n",msg_s->mail->full_address_to);
	SECURE_LOGD("  from: %s\n",msg_s->mail->full_address_from);
	SECURE_LOGD("  cc: %s\n",msg_s->mail->full_address_cc);
	SECURE_LOGD("  bcc: %s\n",msg_s->mail->full_address_bcc);
	SECURE_LOGD("  reply_to: %s\n",msg_s->mail->full_address_reply);
	SECURE_LOGD("  return_path: %s\n",msg_s->mail->full_address_return);
	SECURE_LOGD("  previewBodyText: %s\n",msg_s->mail->preview_text);

	struct_time = localtime(&msg_s->mail->date_time);

	SECURE_LOGD("  %4d year\n", struct_time->tm_year+1900);
	SECURE_LOGD("  %2d month\n", struct_time->tm_mon+1);
	SECURE_LOGD("  %2d day\n", struct_time->tm_mday);
	SECURE_LOGD("  %2d:%2d:%2d \n", struct_time->tm_hour, struct_time->tm_min, struct_time->tm_sec);

	/*--------- body ----------*/
	SECURE_LOGD(" ----------body---------");
	SECURE_LOGD("  body\n");
	SECURE_LOGD("  plain: %s\n",msg_s->mail->file_path_plain);
	SECURE_LOGD("  html: %s\n",msg_s->mail->file_path_html);
	SECURE_LOGD("  attachment_num: %d\n",msg_s->mail->attachment_count);
	tmp_attach = msg_s->attachment;
	for (i=0; i < msg_s->mail->attachment_count; i++) {
		SECURE_LOGD(" ----------attachment[%d]---------", i+1);
		SECURE_LOGD("  name: %s\n",tmp_attach[i].attachment_name);
		SECURE_LOGD("  savename: %s\n",tmp_attach[i].attachment_path);
		SECURE_LOGD("  downloaded: %d\n",tmp_attach[i].save_status);
		SECURE_LOGD("  size: %d\n",tmp_attach[i].attachment_size);
	}		

	/*--------- info ----------*/
	SECURE_LOGD(" ----------info---------");
	SECURE_LOGD("  account_id: %d\n",msg_s->mail->account_id);
	SECURE_LOGD("  mail_id: %d\n",msg_s->mail->mail_id);
	SECURE_LOGD("  mail_size: %d\n",msg_s->mail->mail_size);
	SECURE_LOGD("  body_download_status: %d\n",msg_s->mail->body_download_status);
	SECURE_LOGD("  server_id: %s\n",msg_s->mail->server_mail_id);
	SECURE_LOGD("  meeting_request_status: %d\n",msg_s->mail->meeting_request_status);	

	/*---------- box info -----*/
	box = msg_s->mbox;
	SECURE_LOGD(" ----------box---------");
	SECURE_LOGD("  email_mailbox_t \n");
	SECURE_LOGD("  name: %s\n",box->mailbox_name);
	SECURE_LOGD("  mailbox_type: %d\n",box->mailbox_type);
	SECURE_LOGD("  alias: %s\n",box->alias);
	SECURE_LOGD("  unread_count: %d\n",box->unread_count);
	SECURE_LOGD("  total_mail_count_on_local: %d\n",box->total_mail_count_on_local);
	SECURE_LOGD("  total_mail_count_on_server: %d\n",box->total_mail_count_on_server);
	SECURE_LOGD("  local: %d\n",box->local);
	SECURE_LOGD("  account_id: %d\n",box->account_id);
	SECURE_LOGD("  mail_slot_size: %d\n",box->mail_slot_size);
#endif
	ret=email_add_mail(msg_s->mail, msg_s->attachment, msg_s->mail->attachment_count, NULL, 0);
	ret=CONVERT_ERROR(ret);

	LOGD("END\n");
	return ret;	
}

int email_send_message (email_h msg, bool save_to_sentbox)
{
	LOGD("START\n");
	int ret;
	int handle;

	if (msg == NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}	

	email_s *msg_s = (email_s *)msg;

	ret=email_send_mail(msg_s->mail->mail_id, &handle);

	ret=CONVERT_ERROR(ret);

	LOGD("END\n");
	return ret;
}


email_cb_context * _email_search_callback_by_emailid(int mailid)
{
	LOGD("START\n");
	int count;
	int ntmp=0;
	GSList * node;
	email_cb_context *cbContext;
	count = g_slist_length( gEmailcbList );

	while( count )
	{
		node = g_slist_nth( gEmailcbList, ntmp );
			
		if(node == NULL || node->data == NULL)
			break;
		
		cbContext = (email_cb_context *)node->data;
		if (cbContext->handle->mail->mail_id == mailid) {
			return cbContext;
		}
				
		ntmp++;	
		count--;
	}

	LOGD("END\n");
	return NULL;
}


int email_set_message_sent_cb (email_h handle, email_message_sent_cb cb, void *user_data)
{
	LOGD("START\n");
	int count;
	int ntmp=0;
	int ret = EMAILS_ERROR_NONE;
	GSList * node;
	email_s *p_handle = (email_s *)handle;
	email_cb_context *cbContext;
	count = g_slist_length( gEmailcbList );

	SECURE_LOGD("count : [%d]\n", count);

	if (handle == NULL || p_handle->mail == NULL || cb == NULL) return EMAILS_ERROR_INVALID_PARAMETER;
		
	email_s* msg_s = NULL;
	
	while (count) {
		node = g_slist_nth( gEmailcbList, ntmp );
			
		if (node == NULL)
			break;
		
		cbContext= (email_cb_context *)node->data;
		SECURE_LOGD("mail_id : [%d]\n", cbContext->handle->mail->mail_id);

		if (cbContext->handle->mail->mail_id == p_handle->mail->mail_id)
		{
			gEmailcbList = g_slist_delete_link(gEmailcbList, node);
			_email_free_cb_context(cbContext);	
			break;
		}
				
		ntmp++;	
		count--;
	}

	if ((ret = _email_copy_handle(&msg_s, p_handle)) != EMAILS_ERROR_NONE) {
		SECURE_SLOGE("[%s] _email_copy_handle failed", __FUNCTION__);
		return ret;
	}

	email_cb_context *cbcontext = (email_cb_context *)calloc(1, sizeof(email_cb_context));
	if (cbcontext == NULL) {
		SECURE_SLOGE("[%s] calloc failed", __FUNCTION__);
		ret = EMAIL_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	SECURE_LOGD("mail_id   : [%d]\n", msg_s->mail->mail_id);
	SECURE_LOGD("Callback  : [%p]\n", cb);
	SECURE_LOGD("User Data : [%p]\n", user_data);
	
	cbcontext->handle = msg_s;
	cbcontext->callback = cb;
	cbcontext->user_data = user_data;

	gEmailcbList = g_slist_append(gEmailcbList, cbcontext);

	/* Set the dbus filter */
	_email_add_dbus_filter();

	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int email_unset_message_sent_cb (email_h msg)
{
	LOGD("START\n");
	int i,count;
	count = g_slist_length( gEmailcbList );
	GSList * node;
	email_cb_context *cbContext;

	SECURE_LOGD("count : [%d]\n", count);

	if (msg == NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return EMAILS_ERROR_INVALID_PARAMETER;
	}

	email_s* msg_s = (email_s* )msg;
	for(i=0;i<count;i++)
	{
		node = g_slist_nth( gEmailcbList, i );
		if( node == NULL )
			break;
		
		cbContext= (email_cb_context *)node->data; 
		SECURE_LOGD("mail_id : [%d]\n", cbContext->handle->mail->mail_id);
		SECURE_LOGD("mail_id : [%d]\n", msg_s->mail->mail_id);

		if(cbContext->handle->mail->mail_id == msg_s->mail->mail_id)
		{	
			gEmailcbList = g_slist_delete_link(gEmailcbList, node);
			_email_free_cb_context(cbContext);
			break;
		}
	}

	/* Remove the dbus filter */
	_email_remove_dbus_filter();

	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int _email_error_converter(int err, const char *func, int line)
{
	LOGD("START\n");
	switch(err) 
	{
		case EMAIL_ERROR_INVALID_PARAM :
			SECURE_SLOGE("[%s:%d] INVALID_PARAM(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_INVALID_PARAMETER, err);
			return EMAILS_ERROR_INVALID_PARAMETER;

		case EMAIL_ERROR_DB_FAILURE :
			SECURE_SLOGE("[%s:%d] DB_FAILURE(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_DB_FAILED, err);
			return EMAILS_ERROR_DB_FAILED;

		case EMAIL_ERROR_ACCOUNT_NOT_FOUND :
			SECURE_SLOGE("[%s:%d] ACCOUNT_NOT_FOUND(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_ACCOUNT_NOT_FOUND,err);
			return EMAILS_ERROR_ACCOUNT_NOT_FOUND;

		case EMAIL_ERROR_OUT_OF_MEMORY :
			SECURE_SLOGE("[%s:%d] OUT_OF_MEMORY(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_OUT_OF_MEMORY,err);
			return EMAILS_ERROR_OUT_OF_MEMORY;
			
		// Tizen email F/W  is often using this error type when it gets a null value from server
		//It could be caused from server or IPC.
		case EMAIL_ERROR_NULL_VALUE :
			SECURE_SLOGE("[%s:%d] NULL_VALUE(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_COMMUNICATION_WITH_SERVER_FAILED,err);
			return EMAILS_ERROR_COMMUNICATION_WITH_SERVER_FAILED;

		case EMAIL_ERROR_IPC_SOCKET_FAILURE :
			SECURE_SLOGE("[%s:%d] IPC_SOCKET_FAILURE(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_COMMUNICATION_WITH_SERVER_FAILED,err);
			return EMAILS_ERROR_COMMUNICATION_WITH_SERVER_FAILED;

		case EMAIL_ERROR_PERMISSION_DENIED :
			SECURE_SLOGE("[%s:%d] PERMISSION_DENIED(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_PERMISSION_DENIED,err);
			return EMAILS_ERROR_PERMISSION_DENIED;

		case EMAIL_ERROR_NONE :
			return EMAILS_ERROR_NONE;

		default :
			SECURE_SLOGE("[%s:%d] OPERATION_FAILED(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAILS_ERROR_OPERATION_FAILED,err);
			return EMAILS_ERROR_OPERATION_FAILED;

	}
	LOGD("END\n");
}

static void _monitorSendStatusCb(GDBusConnection *connection, 
									const gchar *sender_name,
									const gchar *object_path,
									const gchar *interface_name,
									const gchar *signal_name,
									GVariant *parameters,
									gpointer data)
{
	LOGD("START\n");
	int status = 0;
	int account_id = 0;
	char *file_id = NULL;
	int mail_id = 0;
	int error_code = 0;

	email_cb_context *cbContext = NULL;

	if (strncasecmp(interface_name, "USER.EMAIL.NETWORKSTATUS", strlen("USER.EMAIL.NETWORKSTATUS")) && 
		strncasecmp(sender_name, "EMAIL", strlen("EMAIL"))) {
		LOGE("Invalid interface : [%s]", interface_name);
		return;
	}

	g_variant_get(parameters, "(iisii)", 
				&status,
				&account_id,
				&file_id,
				&mail_id,
				&error_code);

	SECURE_LOGD("status:[%d], account_id:[%d], file_id:[%s], mail_id:[%d], error_code:[%d]", 
			status, 
			account_id, 
			file_id, 
			mail_id, 
			error_code);

	cbContext = _email_search_callback_by_emailid(mail_id);
	if (cbContext == NULL) {
		SECURE_LOGD("no callback matched!\n");
	} else {
		SECURE_LOGD("Address : [%p]\n", cbContext->callback);
		switch (status) {
		case NOTI_SEND_START:
			break;
			
		case NOTI_SEND_FAIL:
			switch(error_code)
			{
			case EMAIL_ERROR_NO_SIM_INSERTED:
			case EMAIL_ERROR_FLIGHT_MODE:
			case EMAIL_ERROR_SMTP_SEND_FAILURE:
			case EMAIL_ERROR_NO_SUCH_HOST:
			case EMAIL_ERROR_CONNECTION_FAILURE:
			case EMAIL_ERROR_CONNECTION_BROKEN:
			case EMAIL_ERROR_INVALID_SERVER:
			case EMAIL_ERROR_NO_RESPONSE:
				break;

			default:
				break;
			}
			
			cbContext->callback((email_h)cbContext->handle,
								(email_sending_e)EMAIL_SENDING_FAILED,
								cbContext->user_data);
			break;

		case NOTI_SEND_FINISH:
			cbContext->callback((email_h)cbContext->handle,
								(email_sending_e)EMAIL_SENDING_SUCCEEDED,
								cbContext->user_data);
			break;
		}
	}

	LOGE("END\n");
}

void _email_add_dbus_filter(void)
{
	LOGD("START\n");

	if (connection != NULL)
		return;
	
	GError *error = NULL;

	connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (NULL == connection) {
		SECURE_SLOGE("g_bus_get_sync failed : [%s]", error->message);
		g_error_free(error);
		return;
	}
	LOGD("connection [%p]", connection);

	g_dbus_return_id = g_dbus_connection_signal_subscribe(connection, 
													NULL, 
													"User.Email.NetworkStatus", 
													"email", 
													"/User/Email/NetworkStatus",
													NULL,
													G_DBUS_SIGNAL_FLAGS_NONE,
													_monitorSendStatusCb,
													NULL,
													NULL);
	if (g_dbus_return_id == -1) {
		LOGE("g_dbus_connection_signal_subscribe failed");
	}

	LOGE("END\n");
}

void _email_remove_dbus_filter(void)
{
	g_dbus_connection_signal_unsubscribe(connection, g_dbus_return_id);
	g_object_unref(connection);
	connection = NULL;
	g_dbus_return_id = 0;
}

int _email_copy_mail_data(email_mail_data_t **dst_mail_data, email_mail_data_t *src_mail_data)
{
	LOGD("START\n");
	email_mail_data_t *temp_mail_data = NULL;
	
	temp_mail_data = (email_mail_data_t *)calloc(1, sizeof(email_mail_data_t));
	if (temp_mail_data == NULL) {
		SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create email_mail_data_t", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
		return EMAILS_ERROR_OUT_OF_MEMORY;
	}

	temp_mail_data->mail_id		 = src_mail_data->mail_id;
	temp_mail_data->account_id	      = src_mail_data->account_id;
	temp_mail_data->mailbox_id	      = src_mail_data->mailbox_id;
	temp_mail_data->mailbox_type	    = src_mail_data->mailbox_type;
	temp_mail_data->subject		 = EM_SAFE_STRDUP(src_mail_data->subject);
	temp_mail_data->date_time	       = src_mail_data->date_time;
	temp_mail_data->server_mail_status      = src_mail_data->server_mail_status;
	temp_mail_data->server_mailbox_name     = EM_SAFE_STRDUP(src_mail_data->server_mailbox_name);
	temp_mail_data->server_mail_id	  = EM_SAFE_STRDUP(src_mail_data->server_mail_id);
	temp_mail_data->message_id	      = EM_SAFE_STRDUP(src_mail_data->message_id);
	temp_mail_data->full_address_from       = EM_SAFE_STRDUP(src_mail_data->full_address_from);
	temp_mail_data->full_address_reply      = EM_SAFE_STRDUP(src_mail_data->full_address_reply);
	temp_mail_data->full_address_to	 = EM_SAFE_STRDUP(src_mail_data->full_address_to);
	temp_mail_data->full_address_cc	 = EM_SAFE_STRDUP(src_mail_data->full_address_cc);
	temp_mail_data->full_address_bcc	= EM_SAFE_STRDUP(src_mail_data->full_address_bcc);
	temp_mail_data->full_address_return     = EM_SAFE_STRDUP(src_mail_data->full_address_return);
	temp_mail_data->email_address_sender    = EM_SAFE_STRDUP(src_mail_data->email_address_sender);
	temp_mail_data->email_address_recipient = EM_SAFE_STRDUP(src_mail_data->email_address_recipient);
	temp_mail_data->alias_sender	    = EM_SAFE_STRDUP(src_mail_data->alias_sender);
	temp_mail_data->alias_recipient	 = EM_SAFE_STRDUP(src_mail_data->alias_recipient);
	temp_mail_data->body_download_status    = src_mail_data->body_download_status;
	temp_mail_data->file_path_plain	 = EM_SAFE_STRDUP(src_mail_data->file_path_plain);
	temp_mail_data->file_path_html	  = EM_SAFE_STRDUP(src_mail_data->file_path_html);
	temp_mail_data->file_path_mime_entity   = EM_SAFE_STRDUP(src_mail_data->file_path_mime_entity);
	temp_mail_data->mail_size	       = src_mail_data->mail_size;
	temp_mail_data->flags_seen_field	= src_mail_data->flags_seen_field;
	temp_mail_data->flags_deleted_field     = src_mail_data->flags_deleted_field;
	temp_mail_data->flags_flagged_field     = src_mail_data->flags_flagged_field;
	temp_mail_data->flags_answered_field    = src_mail_data->flags_answered_field;
	temp_mail_data->flags_recent_field      = src_mail_data->flags_recent_field;
	temp_mail_data->flags_draft_field       = src_mail_data->flags_draft_field;
	temp_mail_data->flags_forwarded_field   = src_mail_data->flags_forwarded_field;
	temp_mail_data->DRM_status	      = src_mail_data->DRM_status;
	temp_mail_data->priority		= src_mail_data->priority;
	temp_mail_data->save_status	     = src_mail_data->save_status;
	temp_mail_data->lock_status	     = src_mail_data->lock_status;
	temp_mail_data->report_status	   = src_mail_data->report_status;
	temp_mail_data->attachment_count	= src_mail_data->attachment_count;
	temp_mail_data->inline_content_count    = src_mail_data->inline_content_count;
	temp_mail_data->thread_id	       = src_mail_data->thread_id;
	temp_mail_data->thread_item_count       = src_mail_data->thread_item_count;
	temp_mail_data->preview_text	    = EM_SAFE_STRDUP(src_mail_data->preview_text);
	temp_mail_data->meeting_request_status  = src_mail_data->meeting_request_status;
	temp_mail_data->message_class	   = src_mail_data->message_class;
	temp_mail_data->digest_type	     = src_mail_data->digest_type;
	temp_mail_data->smime_type	      = src_mail_data->smime_type;

	*dst_mail_data = temp_mail_data;

	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}

int _email_copy_mailbox(email_mailbox_t **dst_mailbox, email_mailbox_t *src_mailbox)
{
	LOGD("START\n");
	email_mailbox_t *temp_mailbox = NULL;

	temp_mailbox = (email_mailbox_t *)calloc(1,sizeof(email_mailbox_t));
	if (temp_mailbox == NULL)
	{
		SECURE_SLOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create mailbox", __FUNCTION__, EMAILS_ERROR_OUT_OF_MEMORY);
		return EMAILS_ERROR_OUT_OF_MEMORY;
	}

	temp_mailbox->mailbox_id		    = src_mailbox->mailbox_id;
	temp_mailbox->mailbox_name		   = EM_SAFE_STRDUP(src_mailbox->mailbox_name);
	temp_mailbox->mailbox_type		  = src_mailbox->mailbox_type;
	temp_mailbox->alias			 = EM_SAFE_STRDUP(src_mailbox->alias);
	temp_mailbox->unread_count		  = src_mailbox->unread_count;
	temp_mailbox->total_mail_count_on_local     = src_mailbox->total_mail_count_on_local;
	temp_mailbox->total_mail_count_on_server    = src_mailbox->total_mail_count_on_server;
	temp_mailbox->local			 = src_mailbox->local;
	temp_mailbox->account_id		    = src_mailbox->account_id;
	temp_mailbox->mail_slot_size		= src_mailbox->mail_slot_size;
	temp_mailbox->last_sync_time		= src_mailbox->last_sync_time;

	*dst_mailbox = temp_mailbox;

	LOGD("END\n");
	return EMAILS_ERROR_NONE;
}


int _email_copy_handle(email_s **dst_handle, email_s *src_handle)
{
	LOGD("START\n");
	int ret = EMAILS_ERROR_NONE;
	email_s *msg_s = NULL;
	
	msg_s = (email_s *)calloc(1,sizeof(email_s));
	if (msg_s == NULL) {
		SECURE_SLOGE("[%s] calloc failed", __FUNCTION__);
		ret = EMAILS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	if ((ret = _email_copy_mail_data(&msg_s->mail, src_handle->mail)) != EMAILS_ERROR_NONE) {
		SECURE_SLOGE("[%s] _email_copy_mail_data failed", __FUNCTION__);
		email_destroy_message((email_h)msg_s); /*prevent 25431*/
		return ret;
	}

	if ((ret = _email_copy_mailbox(&msg_s->mbox, src_handle->mbox)) != EMAILS_ERROR_NONE) {
		SECURE_SLOGE("[%s] _email_copy_mailbox failed", __FUNCTION__);
		email_destroy_message((email_h)msg_s); /*prevent 25431*/
		return ret;
	}

	*dst_handle = msg_s;
	LOGD("END\n");
	return ret;
}

void _email_free_cb_context(email_cb_context *cbcontext)
{
	LOGD("START\n");
	if(cbcontext == NULL) {
		SECURE_SLOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL.", __FUNCTION__, EMAILS_ERROR_INVALID_PARAMETER);
		return;
	}
		
	email_s* msg_s = cbcontext->handle;

	if(msg_s)
	{
		if (msg_s->mail)
			email_free_mail_data(&msg_s->mail, 1);

		if (msg_s->mbox)
			email_free_mailbox(&msg_s->mbox, 1);

		free(msg_s);		
	}

	cbcontext = NULL;
	LOGD("END\n");
}
