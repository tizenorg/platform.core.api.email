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
#include <E_DBus.h>
#include <email-api-mail.h>
#include <email-api-account.h>
#include <email-api-network.h>
#include <email-api-mailbox.h>
#include <email-api-init.h>
#include <email-api.h>

#define EMAIL_API_ERROR_NONE			1
#define EMAIL_API_ERROR_OUT_OF_MEMORY		-1028
#define EMAIL_API_ERROR_ACCOUNT_NOT_FOUND	-1014

#undef EMAIL_ERROR_NONE
#undef EMAIL_ERROR_OUT_OF_MEMORY
#undef EMAIL_ERROR_ACCOUNT_NOT_FOUND

#include<email.h>
#include<email_private.h>
#include<email_types.h>
#include<email_error.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CAPI_EMAIL"
#define DBG_MODE (1)

#define EM_SAFE_STRDUP(s) \
({\
	char* _s = (char*)s;\
	(_s)? strdup(_s) : NULL;\
})

typedef struct {
	email_message_sent_cb  callback;
	email_s *handle;
	void *user_data;
} email_cb_context;

GSList *gEmailcbList= NULL;

E_DBus_Signal_Handler *handler = NULL;
//------------- Utility Or Miscellaneous
void _email_add_dbus_filter();
int _email_error_converter(int err, const char *func, int line);
int _email_copy_handle(email_s **dst_handle, email_s *src_handle);
void _email_free_cb_context(email_cb_context *cbcontext);

#define CONVERT_ERROR(err) _email_error_converter(err, __FUNCTION__, __LINE__);

//------------------------------------

int email_create_message(email_h *msg)
{
	int ret;
	email_s * msg_s = NULL;
	email_account_t* account = NULL;
	int len;

	if(msg == NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	// 1. create service for ipc
	ret=email_service_begin();
	msg_s= (email_s*)calloc(1,sizeof(email_s));
	if (msg_s != NULL) {
		msg_s->mail = (email_mail_data_t *)calloc(1,sizeof(email_mail_data_t));
		if (msg_s->mail == NULL) {
			LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
			free(msg_s);
			return EMAIL_ERROR_OUT_OF_MEMORY;
		}

		msg_s->mbox = (email_mailbox_t *)calloc(1,sizeof(email_mailbox_t));
		if (msg_s->mbox == NULL) {
			LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mbox", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
			email_free_mail_data(&msg_s->mail, 1);
			free(msg_s);
			return EMAIL_ERROR_OUT_OF_MEMORY;
		}
	} else {
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
	}

	
	//return error from F/W 
	//EMAIL_ERROR_INVALID_PARAM/EMAIL_API_ERROR_NONE/EMAIL_ERROR_DB_FAILURE/EMAIL_ERROR_ACCOUNT_NOT_FOUND/EMAIL_ERROR_OUT_OF_MEMORY
	int default_account_id = 0;
	if ((ret = email_load_default_account_id(&default_account_id)) != EMAIL_API_ERROR_NONE) {
		LOGE("[%s] email_load_default_account_id failed : [%d]",__FUNCTION__, ret);
		email_free_mail_data(&msg_s->mail, 1);
		email_free_mailbox(&msg_s->mbox, 1);
		free(msg_s);
		return CONVERT_ERROR(ret);
	}

	ret = email_get_account(default_account_id, GET_FULL_DATA, &account);
	if(ret!=EMAIL_API_ERROR_NONE) {
		LOGE("[%s] email_get_account failed : [%d]",__FUNCTION__, ret);
		email_free_mail_data(&msg_s->mail, 1);
		email_free_mailbox(&msg_s->mbox, 1);
		free(msg_s);
		return CONVERT_ERROR(ret);
	}

	LOGD_IF(DBG_MODE,"account address = %s",account->user_email_address);
	LOGD_IF(DBG_MODE,"account id = %d",account->account_id);
	LOGD_IF(DBG_MODE,"account name = %s",account->account_name);
	LOGD_IF(DBG_MODE,"account user_name = %s",account->incoming_server_user_name);

	msg_s->mail->full_address_from = (char *)calloc(1,sizeof(char)*(strlen(account->incoming_server_user_name)+strlen(account->user_email_address)+1+1+1+1+1));//"++"+<+ address +> + NULL
	len= (strlen(account->incoming_server_user_name)+strlen(account->user_email_address)+1+1+1+1+1);
	char *strfrom = msg_s->mail->full_address_from;

	snprintf(strfrom,len,"%s%s%s%s%s%s","\"",account->incoming_server_user_name,"\"","<",account->user_email_address,">");

	//mbox
	email_mailbox_t *mbox =msg_s->mbox;

	if ((ret = email_get_mailbox_by_mailbox_type(default_account_id, EMAIL_MAILBOX_TYPE_OUTBOX, &mbox)) != EMAIL_API_ERROR_NONE) {
		LOGE("[%s] email_get_mailbox_by_mailbox_type failed %d", __FUNCTION__, ret);
		email_free_mail_data(&msg_s->mail, 1);
		email_free_mailbox(&msg_s->mbox, 1);
		free(msg_s);
		return EMAIL_ERROR_DB_FAILED;
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
	return EMAIL_ERROR_NONE;
}

int email_destroy_message(email_h msg)
{
	int ret;

	if(msg == NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
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

	if(ret!=EMAIL_API_ERROR_NONE) {
		LOGE("[%s] OPERATION_FAILED(0x%08x) : Finishing email service failed", __FUNCTION__, EMAIL_ERROR_OPERATION_FAILED);
		return EMAIL_ERROR_OPERATION_FAILED;
	}

		
	return EMAIL_ERROR_NONE;
}

int email_set_subject (email_h msg, const char *subject)
{
	int len;
	if(msg ==NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}
	
	email_s* msg_s = (email_s* )msg;	

	msg_s->mail->subject=(char*)calloc(1,sizeof(char)*strlen(subject)+1);

	if(msg_s->mail->subject ==NULL) {
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail->head->subject", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
	}

	len =strlen(subject)+1;
	snprintf(msg_s->mail->subject ,len,"%s",subject); 

	return EMAIL_ERROR_NONE;
}

int email_set_body (email_h msg, const char *body)
{
	int len;

	if (msg == NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	email_s* msg_s = (email_s* )msg;	

	FILE* file = NULL;

	file = fopen("/tmp/capimail.txt", "w");
	if (file != NULL) {
		 fputs(body, file);
		 fclose(file);
	} else {
		LOGE("[%s] OPERATION_FAILED(0x%08x) : opening file for email body failed.", __FUNCTION__, EMAIL_ERROR_OPERATION_FAILED);
		return EMAIL_ERROR_OPERATION_FAILED;
	}

	msg_s->mail->file_path_plain =(char *)calloc(1,sizeof(char)*strlen("/tmp/capimail.txt")+1);

	if(msg_s->mail->file_path_plain == NULL) {
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to allocate body(plain).", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
	}

	len =strlen("/tmp/capimail.txt")+1;
	snprintf(msg_s->mail->file_path_plain,len,"%s","/tmp/capimail.txt");

	return EMAIL_ERROR_NONE;
}

int email_add_recipient (email_h msg, email_recipient_type_e type, const char *address)
{
	char *tmp;
	int total_len,in_len,pre_len,len;

	if(msg == NULL || type < EMAIL_RECIPIENT_TYPE_TO || type > EMAIL_RECIPIENT_TYPE_BCC) {
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	email_s* msg_s = (email_s* )msg;

	if(strlen(address) > MAX_RECIPIENT_ADDRESS_LEN) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : The length of address should be less than 234.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	if(type == EMAIL_RECIPIENT_TYPE_TO) {
		if(msg_s->mail->full_address_to == NULL) {
			msg_s->mail->full_address_to = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(msg_s->mail->full_address_to == NULL)
			{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->to.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				return EMAIL_ERROR_OUT_OF_MEMORY;
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
	} else if (type == EMAIL_RECIPIENT_TYPE_CC) { //MESSAGING_RECIPIENT_TYPE_CC 
		if(msg_s->mail->full_address_cc == NULL) {
			msg_s->mail->full_address_cc = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(msg_s->mail->full_address_cc == NULL) {
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->cc.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				return EMAIL_ERROR_OUT_OF_MEMORY;
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
	} else { //MESSAGING_RECIPIENT_TYPE_BCC
		if (msg_s->mail->full_address_bcc == NULL) {
			msg_s->mail->full_address_bcc = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(msg_s->mail->full_address_bcc==NULL) {
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->bcc.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				return EMAIL_ERROR_OUT_OF_MEMORY;
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

	return EMAIL_ERROR_NONE;
}

int email_remove_all_recipients(email_h msg)
{
	if(msg ==NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : fail to create tmp memory.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	email_s *msg_s = (email_s *)msg;

	if (msg_s->mail->full_address_to != NULL) {
		free(msg_s->mail->full_address_to);
		msg_s->mail->full_address_to = NULL;
	}

	if (msg_s->mail->full_address_cc != NULL) {
		free(msg_s->mail->full_address_cc);
		msg_s->mail->full_address_cc = NULL;
	}

	if (msg_s->mail->full_address_bcc != NULL) {
		free(msg_s->mail->full_address_bcc);
		msg_s->mail->full_address_bcc = NULL;
	}

	return EMAIL_ERROR_NONE;
}

int email_add_attach(email_h msg, const char *filepath)
{
	int len;
	char *pos,*last;
	struct stat st;

	if(msg ==NULL || filepath == NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg or filepath is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	email_s *msg_s = (email_s *)msg;

	int attachment_count = msg_s->mail->attachment_count;
	email_attachment_data_t *new_attach = msg_s->attachment;

	stat(filepath, &st);
	if(st.st_size > 10*1024*1024) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : the size of attachment file is beyond the limit(MAX:10M).", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	if (!S_ISREG(st.st_mode)) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : the filepath is not regular file.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	pos=strpbrk(filepath, "//");
	len =strlen(filepath); 
	if(pos == NULL) {
		new_attach[attachment_count].attachment_name = (char*)calloc(1,sizeof(char)*len+1);
		snprintf(new_attach[attachment_count].attachment_name,len+1,"%s",filepath); 
	} else {
		while(pos != NULL) {
			last=pos;
			pos=strpbrk(pos,"//");	
			if(pos != NULL) pos++;
		}

		new_attach[attachment_count].attachment_name = strdup(last);
	}
	new_attach[attachment_count].attachment_path =(char*)calloc(1,sizeof(char)*len+1);
	if (new_attach[attachment_count].attachment_path == NULL) return EMAIL_ERROR_OUT_OF_MEMORY;

	snprintf(new_attach[attachment_count].attachment_path, len+1, "%s", filepath); 
	new_attach[attachment_count].attachment_size = st.st_size;
	new_attach[attachment_count].save_status = 1;

	msg_s->mail->attachment_count++;

	return EMAIL_ERROR_NONE;
}

int email_remove_all_attachments (email_h msg)
{
	if (msg ==NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	email_s* msg_s = (email_s* )msg;
	msg_s->mail->attachment_count = 0;

	return EMAIL_ERROR_NONE;
}

int email_save_message (email_h msg)
{
	int i, ret;
	email_attachment_data_t *tmp_attach = NULL;
	struct tm *struct_time;

	if (msg == NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}	

	email_s *msg_s = (email_s *)msg;

	/*--------- head ----------*/
	LOGD_IF(DBG_MODE, " ----------head---------");
	LOGD_IF(DBG_MODE, "  mid: %s\n",msg_s->mail->message_id);
	LOGD_IF(DBG_MODE, "  subject: %s\n",msg_s->mail->subject);
	LOGD_IF(DBG_MODE, "  to: %s\n",msg_s->mail->full_address_to);
	LOGD_IF(DBG_MODE, "  from: %s\n",msg_s->mail->full_address_from);
	LOGD_IF(DBG_MODE, "  cc: %s\n",msg_s->mail->full_address_cc);
	LOGD_IF(DBG_MODE, "  bcc: %s\n",msg_s->mail->full_address_bcc);
	LOGD_IF(DBG_MODE, "  reply_to: %s\n",msg_s->mail->full_address_reply);
	LOGD_IF(DBG_MODE, "  return_path: %s\n",msg_s->mail->full_address_return);
	LOGD_IF(DBG_MODE, "  previewBodyText: %s\n",msg_s->mail->preview_text);

	struct_time = localtime(&msg_s->mail->date_time);

	LOGD_IF(DBG_MODE, "  %4d year\n", struct_time->tm_year+1900);
	LOGD_IF(DBG_MODE, "  %2d month\n", struct_time->tm_mon+1);
	LOGD_IF(DBG_MODE, "  %2d day\n", struct_time->tm_mday);
	LOGD_IF(DBG_MODE, "  %2d:%2d:%2d \n", struct_time->tm_hour, struct_time->tm_min, struct_time->tm_sec);

	/*--------- body ----------*/
	LOGD_IF(DBG_MODE, " ----------body---------");
	LOGD_IF(DBG_MODE, "  body\n");
	LOGD_IF(DBG_MODE, "  plain: %s\n",msg_s->mail->file_path_plain);
	LOGD_IF(DBG_MODE, "  html: %s\n",msg_s->mail->file_path_html);
	LOGD_IF(DBG_MODE, "  attachment_num: %d\n",msg_s->mail->attachment_count);
	tmp_attach = msg_s->attachment;
	for (i=0; i < msg_s->mail->attachment_count; i++) {
		LOGD_IF(DBG_MODE, " ----------attachment[%d]---------", i+1);
		LOGD_IF(DBG_MODE, "  name: %s\n",tmp_attach[i].attachment_name);
		LOGD_IF(DBG_MODE, "  savename: %s\n",tmp_attach[i].attachment_path);
		LOGD_IF(DBG_MODE, "  downloaded: %d\n",tmp_attach[i].save_status);
		LOGD_IF(DBG_MODE, "  size: %d\n",tmp_attach[i].attachment_size);
	}		
	
	/*--------- info ----------*/
	LOGD_IF(DBG_MODE, " ----------info---------");
	LOGD_IF(DBG_MODE, "  account_id: %d\n",msg_s->mail->account_id);
	LOGD_IF(DBG_MODE, "  mail_id: %d\n",msg_s->mail->mail_id);
	LOGD_IF(DBG_MODE, "  mail_size: %d\n",msg_s->mail->mail_size);
	LOGD_IF(DBG_MODE, "  body_download_status: %d\n",msg_s->mail->body_download_status);
	LOGD_IF(DBG_MODE, "  server_id: %s\n",msg_s->mail->server_mail_id);
	LOGD_IF(DBG_MODE, "  meeting_request_status: %d\n",msg_s->mail->meeting_request_status);	
	
	email_mailbox_t * box;
	box=msg_s->mbox;
	LOGD_IF(DBG_MODE, " ----------box---------");
	LOGD_IF(DBG_MODE, "  email_mailbox_t \n");
	LOGD_IF(DBG_MODE, "  name: %s\n",box->mailbox_name);
	LOGD_IF(DBG_MODE, "  mailbox_type: %d\n",box->mailbox_type);
	LOGD_IF(DBG_MODE, "  alias: %s\n",box->alias);
	LOGD_IF(DBG_MODE, "  unread_count: %d\n",box->unread_count);
	LOGD_IF(DBG_MODE, "  total_mail_count_on_local: %d\n",box->total_mail_count_on_local);
	LOGD_IF(DBG_MODE, "  total_mail_count_on_server: %d\n",box->total_mail_count_on_server);
	LOGD_IF(DBG_MODE, "  local: %d\n",box->local);
	LOGD_IF(DBG_MODE, "  account_id: %d\n",box->account_id);
	LOGD_IF(DBG_MODE, "  mail_slot_size: %d\n",box->mail_slot_size);

	ret=email_add_mail(msg_s->mail, msg_s->attachment, msg_s->mail->attachment_count, NULL, 0);

	ret=CONVERT_ERROR(ret);

	return ret;	
}

int email_send_message (email_h msg, bool save_to_sentbox)
{
	int ret;
	int handle;

	if (msg == NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}	

	email_s *msg_s = (email_s *)msg;

	ret = email_send_mail(msg_s->mail->mail_id, &handle);

	ret = CONVERT_ERROR(ret);

	_email_add_dbus_filter();

	return ret;
}

email_cb_context *_email_search_callback_by_emailid(int mailid)
{
	int count;
	int ntmp=0;
	GSList *node;
	email_cb_context *cbContext;
	count = g_slist_length(gEmailcbList);

	LOGD_IF(DBG_MODE, "Count : [%d]\n", count);
	while (count) {
		node = g_slist_nth(gEmailcbList, ntmp);
			
		if (node == NULL)
			break;
		
		cbContext = (email_cb_context *)node->data;
		LOGD_IF(DBG_MODE, "mail_id : [%d]\n", cbContext->handle->mail->mail_id);
		if (cbContext->handle->mail->mail_id == mailid) {
			return cbContext;
		}
				
		ntmp++;	
		count--;
	}

	return NULL;
}

int email_set_message_sent_cb (email_h handle, email_message_sent_cb cb, void *user_data)
{
	int count;
	int ntmp=0;
	int ret = EMAIL_ERROR_NONE;
	GSList *node;
	email_cb_context *cbContext;
	count = g_slist_length(gEmailcbList);

	if(handle == NULL || cb == NULL) return EMAIL_ERROR_INVALID_PARAMETER;

	email_s* msg_s = NULL;

	while (count) {
		node = g_slist_nth(gEmailcbList, ntmp);
			
		if (node == NULL)
			break;
		
		cbContext = (email_cb_context *)node->data; 
		if (cbContext->handle == (email_s *)handle) {
			gEmailcbList = g_slist_remove(gEmailcbList,node);
			_email_free_cb_context(cbContext);	
			break;
		}
				
		ntmp++;	
		count--;
	}

	if ((ret = _email_copy_handle(&msg_s, (email_s *)handle)) != EMAIL_ERROR_NONE) {
		LOGE("[%s] _email_copy_handle failed", __FUNCTION__);
		return ret;
	}

	email_cb_context *cbcontext = (email_cb_context *)calloc(1, sizeof(email_cb_context) );

	cbcontext->handle = msg_s;
	cbcontext->callback = cb;
	cbcontext->user_data = user_data;

	gEmailcbList = g_slist_append(gEmailcbList,cbcontext);


	return EMAIL_ERROR_NONE;
}

int email_unset_message_sent_cb (email_h msg)
{
	int i,count;
	count = g_slist_length(gEmailcbList);
	GSList *node;
	email_cb_context *cbContext;

	if (msg ==NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
	}

	email_s* msg_s = (email_s* )msg;
	for (i=0;i<count;i++) {
		node = g_slist_nth(gEmailcbList, i);
		if( node == NULL )
			break;
		
		cbContext = (email_cb_context *)node->data; 
		if(cbContext->handle == msg_s) {	
			gEmailcbList = g_slist_remove(gEmailcbList,node);
			_email_free_cb_context(cbContext);
			break;
		}
	}
	return EMAIL_ERROR_NONE;
}

int _email_error_converter(int err, const char *func, int line)
{
	switch(err) {
	case EMAIL_ERROR_INVALID_PARAM :
		LOGE("[%s:%d] INVALID_PARAM(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAIL_ERROR_INVALID_PARAMETER, err);
		return EMAIL_ERROR_INVALID_PARAMETER;
	case EMAIL_ERROR_DB_FAILURE :
		LOGE("[%s:%d] DB_FAILURE(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAIL_ERROR_DB_FAILED, err);
		return EMAIL_ERROR_DB_FAILED;
	case EMAIL_API_ERROR_ACCOUNT_NOT_FOUND :
		LOGE("[%s:%d] ACCOUNT_NOT_FOUND(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAIL_ERROR_ACCOUNT_NOT_FOUND,err);
		return EMAIL_ERROR_ACCOUNT_NOT_FOUND;
	case EMAIL_API_ERROR_OUT_OF_MEMORY :
		LOGE("[%s:%d] OUT_OF_MEMORY(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAIL_ERROR_OUT_OF_MEMORY,err);
		return EMAIL_ERROR_OUT_OF_MEMORY;
	// Tizen email F/W  is often using this error type when it gets a null value from server
	//It could be caused from server or IPC.
	case EMAIL_ERROR_NULL_VALUE :
		LOGE("[%s:%d] NULL_VALUE(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAIL_ERROR_COMMUNICATION_WITH_SERVER_FAILED,err);
		return EMAIL_ERROR_COMMUNICATION_WITH_SERVER_FAILED;
	case EMAIL_ERROR_IPC_SOCKET_FAILURE :
		LOGE("[%s:%d] IPC_SOCKET_FAILURE(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAIL_ERROR_COMMUNICATION_WITH_SERVER_FAILED,err);
		return EMAIL_ERROR_COMMUNICATION_WITH_SERVER_FAILED;
	case EMAIL_API_ERROR_NONE :
		return EMAIL_ERROR_NONE;
	default :
		LOGE("[%s:%d] OPERATION_FAILED(0x%08x) : Error from Email F/W. ret: (0x%08x) ", func, line, EMAIL_ERROR_OPERATION_FAILED,err);
		return EMAIL_ERROR_OPERATION_FAILED;
	}
}

static void _monitorSendStatusCb(void* data, DBusMessage *message)
{
	DBusError error;
	email_cb_context *cbContext = NULL;
	if (dbus_message_is_signal(message, "User.Email.NetworkStatus", "email")) {
		dbus_error_init(&error);

		int status=0;
		int accountid=0;
		char* fileid=NULL;
		int mailid=0;
		int errorcode=0;
		if(dbus_message_get_args(message, &error, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &accountid, DBUS_TYPE_STRING, &fileid, DBUS_TYPE_INT32, &mailid, DBUS_TYPE_INT32, &errorcode, DBUS_TYPE_INVALID)) {
			LOGD_IF(DBG_MODE, "status:[%d], account_id:[%d], file_id:[%s], mail_id:[%d], error_code:[%d]", status, accountid, fileid, mailid, errorcode);
			cbContext = _email_search_callback_by_emailid(mailid);
			if(cbContext == NULL) {
					LOGD_IF(DBG_MODE, "no callback matched!\n");
			} else {
				switch (status) {
				case NOTI_SEND_START:
					break;
				case NOTI_SEND_FAIL:
					switch(errorcode) {
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
					cbContext->callback((email_h)cbContext->handle, (email_sending_e)EMAIL_SENDING_FAILED, cbContext->user_data);
						break;
				case NOTI_SEND_FINISH:
					cbContext->callback((email_h)cbContext->handle, (email_sending_e)EMAIL_SENDING_SUCCEEDED, cbContext->user_data);
					break;
				}
			}
		}
	}
}

void _email_add_dbus_filter()
{
	E_DBus_Connection* connection=NULL;
	DBusError error;

	if (handler != NULL)
		return;

	dbus_error_init(&error);

	connection = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (NULL == connection) {
		return;
	}

	if (e_dbus_request_name(connection, "User.Email.NetworkStatus", 0, NULL, NULL) == NULL) {
		LOGD_IF(DBG_MODE, "Failed in e_dbus_request_name()\n");
		return;
	}

	handler = e_dbus_signal_handler_add(connection, NULL, "/User/Email/NetworkStatus", "User.Email.NetworkStatus", "email", _monitorSendStatusCb, NULL);

	if (handler == NULL) {
		LOGD_IF(DBG_MODE, "Failed in e_dbus_signal_handler_add()");
	}
}

int _email_copy_mail_data(email_mail_data_t **dst_mail_data, email_mail_data_t *src_mail_data)
{
	email_mail_data_t *temp_mail_data = NULL;

	temp_mail_data = (email_mail_data_t *)calloc(1, sizeof(email_mail_data_t));
	if (temp_mail_data == NULL) {
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create email_mail_data_t", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
	}

	temp_mail_data->mail_id                 = src_mail_data->mail_id;
	temp_mail_data->account_id              = src_mail_data->account_id;
	temp_mail_data->mailbox_id              = src_mail_data->mailbox_id;
	temp_mail_data->mailbox_type            = src_mail_data->mailbox_type;
	temp_mail_data->subject                 = EM_SAFE_STRDUP(src_mail_data->subject);
	temp_mail_data->date_time               = src_mail_data->date_time;
	temp_mail_data->server_mail_status      = src_mail_data->server_mail_status;
	temp_mail_data->server_mailbox_name     = EM_SAFE_STRDUP(src_mail_data->server_mailbox_name);
	temp_mail_data->server_mail_id          = EM_SAFE_STRDUP(src_mail_data->server_mail_id);
	temp_mail_data->message_id              = EM_SAFE_STRDUP(src_mail_data->message_id);
	temp_mail_data->full_address_from       = EM_SAFE_STRDUP(src_mail_data->full_address_from);
	temp_mail_data->full_address_reply      = EM_SAFE_STRDUP(src_mail_data->full_address_reply);
	temp_mail_data->full_address_to         = EM_SAFE_STRDUP(src_mail_data->full_address_to);
	temp_mail_data->full_address_cc         = EM_SAFE_STRDUP(src_mail_data->full_address_cc);
	temp_mail_data->full_address_bcc        = EM_SAFE_STRDUP(src_mail_data->full_address_bcc);
	temp_mail_data->full_address_return     = EM_SAFE_STRDUP(src_mail_data->full_address_return);
	temp_mail_data->email_address_sender    = EM_SAFE_STRDUP(src_mail_data->email_address_sender);
	temp_mail_data->email_address_recipient = EM_SAFE_STRDUP(src_mail_data->email_address_recipient);
	temp_mail_data->alias_sender            = EM_SAFE_STRDUP(src_mail_data->alias_sender);
	temp_mail_data->alias_recipient         = EM_SAFE_STRDUP(src_mail_data->alias_recipient);
	temp_mail_data->body_download_status    = src_mail_data->body_download_status;
	temp_mail_data->file_path_plain         = EM_SAFE_STRDUP(src_mail_data->file_path_plain);
	temp_mail_data->file_path_html          = EM_SAFE_STRDUP(src_mail_data->file_path_html);
	temp_mail_data->file_path_mime_entity   = EM_SAFE_STRDUP(src_mail_data->file_path_mime_entity);
	temp_mail_data->mail_size               = src_mail_data->mail_size;
	temp_mail_data->flags_seen_field        = src_mail_data->flags_seen_field;
	temp_mail_data->flags_deleted_field     = src_mail_data->flags_deleted_field;
	temp_mail_data->flags_flagged_field     = src_mail_data->flags_flagged_field;
	temp_mail_data->flags_answered_field    = src_mail_data->flags_answered_field;
	temp_mail_data->flags_recent_field      = src_mail_data->flags_recent_field;
	temp_mail_data->flags_draft_field       = src_mail_data->flags_draft_field;
	temp_mail_data->flags_forwarded_field   = src_mail_data->flags_forwarded_field;
	temp_mail_data->DRM_status              = src_mail_data->DRM_status;
	temp_mail_data->priority                = src_mail_data->priority;
	temp_mail_data->save_status             = src_mail_data->save_status;
	temp_mail_data->lock_status             = src_mail_data->lock_status;
	temp_mail_data->report_status           = src_mail_data->report_status;
	temp_mail_data->attachment_count        = src_mail_data->attachment_count;
	temp_mail_data->inline_content_count    = src_mail_data->inline_content_count;
	temp_mail_data->thread_id               = src_mail_data->thread_id;
	temp_mail_data->thread_item_count       = src_mail_data->thread_item_count;
	temp_mail_data->preview_text            = EM_SAFE_STRDUP(src_mail_data->preview_text);
	temp_mail_data->meeting_request_status  = src_mail_data->meeting_request_status;
	temp_mail_data->message_class           = src_mail_data->message_class;
	temp_mail_data->digest_type             = src_mail_data->digest_type;
	temp_mail_data->smime_type              = src_mail_data->smime_type;

	*dst_mail_data = temp_mail_data;

	return EMAIL_ERROR_NONE;
}

int _email_copy_mailbox(email_mailbox_t **dst_mailbox, email_mailbox_t *src_mailbox)
{
	email_mailbox_t *temp_mailbox = NULL;

	temp_mailbox = (email_mailbox_t *)calloc(1,sizeof(email_mailbox_t));
	if (temp_mailbox == NULL) {
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create mailbox", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
	}

	temp_mailbox->mailbox_id                    = src_mailbox->mailbox_id;
	temp_mailbox->mailbox_name                   = EM_SAFE_STRDUP(src_mailbox->mailbox_name);
	temp_mailbox->mailbox_type                  = src_mailbox->mailbox_type;
	temp_mailbox->alias                         = EM_SAFE_STRDUP(src_mailbox->alias);
	temp_mailbox->unread_count                  = src_mailbox->unread_count;
	temp_mailbox->total_mail_count_on_local     = src_mailbox->total_mail_count_on_local;
	temp_mailbox->total_mail_count_on_server    = src_mailbox->total_mail_count_on_server;
	temp_mailbox->local                         = src_mailbox->local;
	temp_mailbox->account_id                    = src_mailbox->account_id;
	temp_mailbox->mail_slot_size                = src_mailbox->mail_slot_size;
	temp_mailbox->last_sync_time                = src_mailbox->last_sync_time;

	*dst_mailbox = temp_mailbox;

	return EMAIL_ERROR_NONE;
}

int _email_copy_handle(email_s **dst_handle, email_s *src_handle)
{
	int ret = EMAIL_ERROR_NONE;
	email_s *msg_s = NULL;

	msg_s = (email_s *)calloc(1,sizeof(email_s));
	if ((ret = _email_copy_mail_data(&msg_s->mail, src_handle->mail)) != EMAIL_ERROR_NONE) {
		LOGE("[%s] _email_copy_mail_data failed", __FUNCTION__);
		email_destroy_message((email_h)msg_s); /*prevent 25431*/
		return ret;
	}

	if ((ret = _email_copy_mailbox(&msg_s->mbox, src_handle->mbox)) != EMAIL_ERROR_NONE) {
		LOGE("[%s] _email_copy_mailbox failed", __FUNCTION__);
		email_destroy_message((email_h)msg_s); /*prevent 25431*/
		return ret;
	}

	*dst_handle = msg_s;
	return ret;
}

void _email_free_cb_context(email_cb_context *cbcontext)
{
	if(cbcontext == NULL) {
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
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
}
