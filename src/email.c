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


#include <memory.h>
#include <dlog.h>
#include <E_DBus.h>
#include <emf-types.h>
#include <Emf_Mapi_Account.h>
#include <Emf_Mapi_Message.h>
#include <Emf_Mapi_Network.h>
#include <Emf_Mapi_Mailbox.h>
#include <Emf_Mapi_Init.h>

#include<email.h>
#include<email_private.h>
#include<email_types.h>
#include<email_error.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CAPI_EMAIL"
#define DBG_MODE (0)

typedef struct {
	email_message_sent_cb  callback;
	email_s *handle;
	void *user_data;
} email_cb_context;

GSList *gEmailcbList= NULL;
//------------- Utility Or Miscellaneous
void _email_add_dbus_filter(void);
int _email_error_converter(int err);

//------------------------------------

int  email_create_message(email_h *msg)
{
	int ret;
	email_s * msg_s = NULL;
	emf_account_t* account = NULL;
	int i,cnt,len;

	// 1. create service for ipc
	ret=email_service_begin();
	msg_s= (email_s*)calloc(1,sizeof(email_s));
	if (msg_s != NULL)
	{
		msg_s->mail = (emf_mail_t *)calloc(1,sizeof(emf_mail_t));
		if (msg_s->mail != NULL)
		{
		
			msg_s->mail->head = (emf_mail_head_t *)calloc(1,sizeof(emf_mail_head_t));
			if(msg_s->mail->head == NULL)
			{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail->head", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				free(msg_s);
				return EMAIL_ERROR_OUT_OF_MEMORY;
			}

			msg_s->mail->body = (emf_mail_body_t *)calloc(1,sizeof(emf_mail_body_t));
			if (msg_s->mail->body  == NULL)
			{
			 	LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail->body", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				free(msg_s);
				return EMAIL_ERROR_OUT_OF_MEMORY;
			}


			msg_s->mail->info = (emf_mail_info_t *)calloc(1,sizeof(emf_mail_info_t));
			if (msg_s->mail->info  == NULL)
			{
			   	LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail->info", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				free(msg_s);
				return EMAIL_ERROR_OUT_OF_MEMORY;
			}
		}
		else
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
			free(msg_s);
			return EMAIL_ERROR_OUT_OF_MEMORY;
			}

		msg_s->mbox = (emf_mailbox_t *)calloc(1,sizeof(emf_mailbox_t));
		if (msg_s->mbox == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mbox", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
			free(msg_s);
			return EMAIL_ERROR_OUT_OF_MEMORY;
			}
		
	
	}
	else
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
		}

		
	//return error from F/W 
	//EMF_ERROR_INVALID_PARAM/EMF_ERROR_NONE/EMF_ERROR_DB_FAILURE/EMF_ERROR_ACCOUNT_NOT_FOUND/EMF_ERROR_OUT_OF_MEMORY
	ret=email_get_account_list(&account, &cnt);
	
	
	if(ret!=EMF_ERROR_NONE) return _email_error_converter(ret);

	for(i=0;i<cnt;i++)
	{
		LOGD_IF(DBG_MODE,"account address[%d]= %s",i,account[i].email_addr);
		LOGD_IF(DBG_MODE,"account id[%d]= %d",i,account[i].account_id);
		LOGD_IF(DBG_MODE,"account name[%d]= %s",i,account[i].account_name);
		LOGD_IF(DBG_MODE,"account user_name[%d]= %s",i,account[i].user_name);

		}

	emf_mailbox_t* mailbox_list = NULL;
	int sync_type =1;
	email_get_mailbox_list(account[0].account_id, sync_type, &mailbox_list, &cnt);

	
	msg_s->mail->head->from = (char *)calloc(1,sizeof(char)*(strlen(account[0].user_name)+strlen(account[0].email_addr)+1+1+1+1+1));//"++"+<+ address +> + NULL
	len= (strlen(account[0].user_name)+strlen(account[0].email_addr)+1+1+1+1+1);
	char *strfrom = msg_s->mail->head->from;

	snprintf(strfrom,len,"%s%s%s%s%s%s","\"",account[0].user_name,"\"","<",account[0].email_addr,">");


	//info
	emf_mail_info_t *info = msg_s->mail->info;
	info->account_id = account[0].account_id;
	info->flags.draft = 1;
	
	//set flag1 as seen
	info->flags.seen = 1;

	info->extra_flags.priority = EMF_MAIL_PRIORITY_NORMAL;

	//mbox
	emf_mailbox_t * mbox =msg_s->mbox;
	

	mbox->name  = (char *)calloc(1,sizeof(char)*strlen("OUTBOX")+1);

	if(mbox->name ==NULL){
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create mbox->name", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
		}
	
	len = strlen("OUTBOX")+1;
	snprintf(mbox->name,len,"%s","OUTBOX");
	mbox->mailbox_type = EMF_MAILBOX_TYPE_OUTBOX;
	

	mbox->alias  = (char *)calloc(1,sizeof(char)*strlen("Outbox")+1);

	if(mbox->alias ==NULL){
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create mbox->alias", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
		}
	
	len = strlen("Outbox")+1;
	snprintf(mbox->alias,len,"%s","Outbox");
	
	mbox->local = 1;
	mbox->synchronous = 1;
	mbox->account_id = account[0].account_id;
	mbox->next = NULL;
	mbox->mail_slot_size = 50;
	

	*msg = (email_h)msg_s;
	return EMAIL_ERROR_NONE;
}

int email_destroy_message(email_h msg)
{
	int ret;


	if(msg ==NULL ){
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}
		
	email_s* msg_s = (email_s* )msg;

	if(msg_s )
	{
		if(msg_s->mail)
		{
			if(msg_s->mail->head)free(msg_s->mail->head);

			if(msg_s->mail->body)
			{
				if(msg_s->mail->body->plain)remove(msg_s->mail->body->plain);				
				free(msg_s->mail->body);
			}

			if(msg_s->mail->info)free(msg_s->mail->info);
			free(msg_s->mail);
		}

		if(msg_s->mbox)free(msg_s->mbox);

		free(msg_s);		
	}

	
	ret=email_service_end();
	
	if(ret!=EMF_ERROR_NONE){
		LOGE("[%s] OPERATION_FAILED(0x%08x) : Finishing email service failed", __FUNCTION__, EMAIL_ERROR_OPERATION_FAILED);
		return EMAIL_ERROR_OPERATION_FAILED;
		}
	
		
	return EMAIL_ERROR_NONE;
}

int email_set_subject (email_h msg, const char *subject)
{
	int len;
	if(msg ==NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is NULL", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}
		
	email_s* msg_s = (email_s* )msg;	

	msg_s->mail->head->subject=(char*)calloc(1,sizeof(char)*strlen(subject)+1);

	if(msg_s->mail->head->subject ==NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create msg_s->mail->head->subject", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
		return EMAIL_ERROR_OUT_OF_MEMORY;
		}
	
	len =strlen(subject)+1;
	snprintf(msg_s->mail->head->subject ,len,"%s",subject); 
	
	return EMAIL_ERROR_NONE;
}

int email_set_body (email_h msg, const char *body)
{
	int len;

	if(msg ==NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}
	
	email_s* msg_s = (email_s* )msg;	


	FILE* file = NULL;

	
       file= fopen("/tmp/capimail.txt", "w");

	if(file !=NULL)
	{
		 fputs(body,file);
		 fclose(file);
	}else
	{
	   	LOGE("[%s] OPERATION_FAILED(0x%08x) : opening file for email body failed.", __FUNCTION__, EMAIL_ERROR_OPERATION_FAILED);
		return EMAIL_ERROR_OPERATION_FAILED;
	}



	msg_s->mail->body->plain =(char *)calloc(1,sizeof(char)*strlen("/tmp/capimail.txt")+1);

	if(msg_s->mail->body->plain==NULL)return EMAIL_ERROR_OUT_OF_MEMORY;
	
	len =strlen("/tmp/capimail.txt")+1;
	snprintf(msg_s->mail->body->plain,len,"%s","/tmp/capimail.txt");
	
	return EMAIL_ERROR_NONE;
}

 

int email_add_recipient (email_h msg, email_recipient_type_e type,const char *address)
{
	

	char *tmp,*tmp1;
	int total_len,in_len,pre_len,len;

	if(msg ==NULL || type<EMAIL_RECIPIENT_TYPE_TO ||type>EMAIL_RECIPIENT_TYPE_BCC )
		return EMAIL_ERROR_INVALID_PARAMETER;
		
	
	email_s* msg_s = (email_s* )msg;
	emf_mail_head_t *head = msg_s->mail->head;
	
	if(strlen(address)>MAX_RECIPIENT_ADDRESS_LEN)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : The length of address should be less than 234.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}

	if(type == EMAIL_RECIPIENT_TYPE_TO)
	{
		if(head->to==NULL)
		{
			head->to = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(head->to==NULL)
			{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->to.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
			 	return EMAIL_ERROR_OUT_OF_MEMORY;
				}
			len =strlen(address)+2+1+1;
			snprintf(head->to,len,"%s%s%s","<",address,">");
		}
		else{

			in_len = strlen(address);
			pre_len = strlen(head->to);
			total_len = pre_len+in_len+3+1;// length of ",<>" + NULL
			tmp=(char*)calloc(1,sizeof(char)*total_len);

			//remove ';'
			tmp1 =(char*)calloc(1,sizeof(char)*pre_len+1);
			if(tmp1)
			{
				strncpy(tmp1,head->to,pre_len-1);
				tmp1[pre_len]='\0';
				}
			else
			{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create tmp memory.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				if(tmp)free(tmp);
				return EMAIL_ERROR_OUT_OF_MEMORY;
				}

			//add new address
			free(head->to);
			head->to = (char*)calloc(1,sizeof(char)*total_len);
			len = total_len;
			snprintf(head->to,len,"%s%s%s%s",tmp1,"<",address,">");
			
			if(tmp)free(tmp);
			if(tmp1)free(tmp1);
			
		}
		
	}
	else if(type == EMAIL_RECIPIENT_TYPE_CC)//MESSAGING_RECIPIENT_TYPE_CC
	{
		if(head->cc==NULL)
		{
			head->cc = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(head->cc==NULL)
			{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->cc.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				return EMAIL_ERROR_OUT_OF_MEMORY;
				}
			len =strlen(address)+2+1+1;
			snprintf(head->cc,len,"%s%s%s","<",address,">");
		}
		else{

			in_len = strlen(address);
			pre_len = strlen(head->cc);
			total_len = pre_len+in_len+3+1;// length of ",<>" + NULL
			tmp=(char*)calloc(1,sizeof(char)*total_len);

			//remove ';'
			tmp1 =(char*)calloc(1,sizeof(char)*pre_len+1);
			if(tmp1)
			{
				strncpy(tmp1,head->cc,pre_len-1);
				tmp1[pre_len]='\0';
				}
			else{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create tmp memory.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				if(tmp)free(tmp);
				return EMAIL_ERROR_OUT_OF_MEMORY;
			}

			//add new address
			free(head->cc);
			head->cc = (char*)calloc(1,sizeof(char)*total_len);
			len = total_len;
			snprintf(head->cc,len,"%s%s%s%s",tmp1,"<",address,">"); 

			if(tmp)free(tmp);
			if(tmp1)free(tmp1);
		}
		
	}
	else //MESSAGING_RECIPIENT_TYPE_BCC
	{
		if(head->bcc==NULL)
		{
			head->bcc = (char*)calloc(1,sizeof(char)*strlen(address)+2+1+1);//<>+;+end of string
			if(head->bcc==NULL)
			{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create head->bcc.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				return EMAIL_ERROR_OUT_OF_MEMORY;
				}
			len =strlen(address)+2+1+1;
			snprintf(head->bcc,len,"%s%s%s","<",address,">");
		}
		else{

			in_len = strlen(address);
			pre_len = strlen(head->bcc);
			total_len = pre_len+in_len+3+1;// length of ",<>" + NULL
			tmp=(char*)calloc(1,sizeof(char)*total_len);

			//remove ';'
			tmp1 =(char*)calloc(1,sizeof(char)*pre_len+1);
			if(tmp1)
			{
				strncpy(tmp1,head->bcc,pre_len-1);
				tmp1[pre_len]='\0';
				}
			else{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x) : fail to create tmp memory.", __FUNCTION__, EMAIL_ERROR_OUT_OF_MEMORY);
				if(tmp)free(tmp);
				return EMAIL_ERROR_OUT_OF_MEMORY;
				}

			//add new address
			free(head->bcc);
			head->bcc = (char*)calloc(1,sizeof(char)*total_len);
			len = total_len;
			snprintf(head->bcc,len,"%s%s%s%s",tmp1,"<",address,">"); 

			if(tmp)free(tmp);
			if(tmp1)free(tmp1);
		}
	}

	

	return EMAIL_ERROR_NONE;
}

int email_remove_all_recipients (email_h msg)
{
	if(msg ==NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : fail to create tmp memory.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}
	
	email_s* msg_s = (email_s* )msg;

	emf_mail_head_t *head = msg_s->mail->head;
	if(head->to !=NULL)
	{
		free(head->to);
		head->to = NULL;
	}

	if(head->cc !=NULL)
	{
		free(head->cc);
		head->cc = NULL;
	}

	if(head->bcc !=NULL)
	{
		free(head->bcc);
		head->bcc = NULL;
	}
	return EMAIL_ERROR_NONE;
}


int email_add_attach (email_h msg, const char *filepath)
{
	int i,len;
	char *pos,*last;
	struct stat st;
	

	if(msg ==NULL ||filepath == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg or filepath is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}
		
	email_s* msg_s = (email_s* )msg;

	emf_attachment_info_t*  new_attach,* tmp_attach,* next_attach;
	emf_mail_body_t* body = msg_s->mail->body;	


	stat(filepath,&st);

	if(st.st_size > 10*1024*1024)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : the size of attachment file is beyond the limit(MAX:10M).", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}
	
	new_attach =(emf_attachment_info_t *)calloc(1,sizeof(emf_attachment_info_t));


	pos=strpbrk(filepath,"//");

	len =strlen(filepath); 
	
	if(pos==NULL)
	{
		new_attach->name =(char*)calloc(1,sizeof(char)*len+1);
		snprintf(new_attach->name,len,"%s",filepath); 
		}
	else
	{
		while(pos!=NULL)
		{
			last=pos;
			pos=strpbrk(pos,"//");	
			if(pos!=NULL)pos++;
			}
		
		new_attach->name =last;
		
		}
	new_attach->savename =(char*)calloc(1,sizeof(char)*len+1);
	
	if(new_attach->savename==NULL)return EMAIL_ERROR_OUT_OF_MEMORY;
	
	snprintf(new_attach->savename,len,"%s",filepath); 
	new_attach->size = st.st_size;
	new_attach->downloaded = 1;

	if(body->attachment_num ==0){

		body->attachment = new_attach;
	}	
	else{
		next_attach=body->attachment;
		
		for(i=0;i<body->attachment_num;i++)
		{
			tmp_attach =next_attach->next;
			next_attach =tmp_attach;
		}
	}

	body->attachment_num ++;

	return EMAIL_ERROR_NONE;

}

int email_remove_all_attachments (email_h msg)
{
	if(msg ==NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}	
	email_s* msg_s = (email_s* )msg;
	emf_mail_body_t* body = msg_s->mail->body;
	if(body->attachment != NULL)
	{
		free(body->attachment);
		}
	return EMAIL_ERROR_NONE;
}


int email_send_message (email_h msg)
{
	int ret;
	emf_option_t option;
	unsigned  handle;

	if(msg ==NULL )
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}	
		
	email_s* msg_s = (email_s* )msg;

	
	{
		/*--------- head ----------*/
		LOGD_IF(DBG_MODE, " ----------head---------");
		LOGD_IF(DBG_MODE, "  mid: %s\n",msg_s->mail->head->mid);
		LOGD_IF(DBG_MODE, "  subject: %s\n",msg_s->mail->head->subject);
		LOGD_IF(DBG_MODE, "  to: %s\n",msg_s->mail->head->to);
		LOGD_IF(DBG_MODE, "  from: %s\n",msg_s->mail->head->from);
		LOGD_IF(DBG_MODE, "  cc: %s\n",msg_s->mail->head->cc);
		LOGD_IF(DBG_MODE, "  bcc: %s\n",msg_s->mail->head->bcc);
		LOGD_IF(DBG_MODE, "  reply_to: %s\n",msg_s->mail->head->reply_to);
		LOGD_IF(DBG_MODE, "  return_path: %s\n",msg_s->mail->head->return_path);
		LOGD_IF(DBG_MODE, "  from_contact_name: %s\n",msg_s->mail->head->from_contact_name);
		LOGD_IF(DBG_MODE, "  to_contact_name: %s\n",msg_s->mail->head->to_contact_name);
		LOGD_IF(DBG_MODE, "  cc_contact_name: %s\n",msg_s->mail->head->cc_contact_name);
		LOGD_IF(DBG_MODE, "  bcc_contact_name: %s\n",msg_s->mail->head->bcc_contact_name);
		LOGD_IF(DBG_MODE, "  previewBodyText: %s\n",msg_s->mail->head->previewBodyText);

		LOGD_IF(DBG_MODE, "  year: %d\n",msg_s->mail->head->datetime.year);
		LOGD_IF(DBG_MODE, "  month: %d\n",msg_s->mail->head->datetime.month);
		LOGD_IF(DBG_MODE, "  day: %d\n",msg_s->mail->head->datetime.day);
		LOGD_IF(DBG_MODE, "  hour: %d\n",msg_s->mail->head->datetime.hour);
		LOGD_IF(DBG_MODE, "  minute: %d\n",msg_s->mail->head->datetime.minute);
		LOGD_IF(DBG_MODE, "  second: %d\n",msg_s->mail->head->datetime.second);

		/*--------- body ----------*/
		LOGD_IF(DBG_MODE, " ----------body---------");
		LOGD_IF(DBG_MODE, "  body\n");
		LOGD_IF(DBG_MODE, "  plain: %s\n",msg_s->mail->body->plain);
		LOGD_IF(DBG_MODE, "  plain_charset: %s\n",msg_s->mail->body->plain_charset);
		LOGD_IF(DBG_MODE, "  html: %s\n",msg_s->mail->body->html);
		LOGD_IF(DBG_MODE, "  attachment_num: %d\n",msg_s->mail->body->attachment_num);
		if(msg_s->mail->body->attachment!=NULL)
		{
			LOGD_IF(DBG_MODE, " ----------attachment---------");
			LOGD_IF(DBG_MODE, "  name: %s\n",msg_s->mail->body->attachment->name);
			LOGD_IF(DBG_MODE, "  savename: %s\n",msg_s->mail->body->attachment->savename);
			LOGD_IF(DBG_MODE, "  downloaded: %d\n",msg_s->mail->body->attachment->downloaded);
			LOGD_IF(DBG_MODE, "  size: %d\n",msg_s->mail->body->attachment->size);
			}
	
	}

	{
			/*--------- info ----------*/
		LOGD_IF(DBG_MODE, " ----------info---------");
		LOGD_IF(DBG_MODE, "  emf_mail_info_t \n");
		LOGD_IF(DBG_MODE, "  account_id: %d\n",msg_s->mail->info->account_id);
		LOGD_IF(DBG_MODE, "  uid: %d\n",msg_s->mail->info->uid);
		LOGD_IF(DBG_MODE, "  rfc822_size: %d\n",msg_s->mail->info->rfc822_size);
		LOGD_IF(DBG_MODE, "  body_downloaded: %d\n",msg_s->mail->info->body_downloaded);
		LOGD_IF(DBG_MODE, "  sid: %s\n",msg_s->mail->info->sid);
		LOGD_IF(DBG_MODE, "   is_meeting_request: %d\n",msg_s->mail->info->is_meeting_request);	
	}

	
	{
		emf_mailbox_t * box;
		box=msg_s->mbox;
		LOGD_IF(DBG_MODE, " ----------box---------");
		LOGD_IF(DBG_MODE, "  emf_mailbox_t \n");
		LOGD_IF(DBG_MODE, "  name: %s\n",box->name);
		LOGD_IF(DBG_MODE, "  mailbox_type: %d\n",box->mailbox_type);
		LOGD_IF(DBG_MODE, "  alias: %s\n",box->alias);
		LOGD_IF(DBG_MODE, "  unread_count: %d\n",box->unread_count);
		LOGD_IF(DBG_MODE, "  total_mail_count_on_local: %d\n",box->total_mail_count_on_local);
		LOGD_IF(DBG_MODE, "  total_mail_count_on_server: %d\n",box->total_mail_count_on_server);
		LOGD_IF(DBG_MODE, "  local: %d\n",box->local);
		LOGD_IF(DBG_MODE, "  synchronous: %d\n",box->synchronous);
		LOGD_IF(DBG_MODE, "  account_id: %d\n",box->account_id);
		LOGD_IF(DBG_MODE, "  has_archived_mails: %d\n",box->has_archived_mails);
		LOGD_IF(DBG_MODE, "  mail_slot_size: %d\n",box->mail_slot_size);
		LOGD_IF(DBG_MODE, "  account_name: %s\n",box->account_name);
	}


	ret=email_add_message(msg_s->mail, msg_s->mbox, 1);

	ret=_email_error_converter(ret);
	
	option.keep_local_copy = 1;

	ret=email_send_mail(msg_s->mbox, msg_s->mail->info->uid, &option, &handle);


	ret=_email_error_converter(ret);
	_email_add_dbus_filter();
	return ret;
}


email_cb_context * _email_search_callback_by_emailid(int mailid)
{
	int count;
	int ntmp=0;
	GSList * node;
	email_cb_context *cbContext;
	count = g_slist_length( gEmailcbList );

	while( count )
	{
		node = g_slist_nth( gEmailcbList, ntmp );
			
			if( node == NULL )
				break;
			
			cbContext= (email_cb_context *)node->data; 
			if(cbContext->handle->mail->info->uid == mailid)
			{
				
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
	GSList * node;
	email_cb_context *cbContext;
	count = g_slist_length( gEmailcbList );

	if(handle ==NULL || cb == NULL)return EMAIL_ERROR_INVALID_PARAMETER;
		
	
	email_s* msg_s = (email_s* )handle;
	
	while( count )
	{
		node = g_slist_nth( gEmailcbList, ntmp );
			
			if( node == NULL )
				break;
			
			cbContext= (email_cb_context *)node->data; 
			if(cbContext->handle == (email_s*)handle)
			{
					gEmailcbList=g_slist_remove(gEmailcbList,node);
					break;
			}
				
				
		ntmp++;	
		count--;
	}

	email_cb_context * cbcontext= (email_cb_context*)calloc(1, sizeof(email_cb_context) );
	
	cbcontext->handle = msg_s;
	cbcontext->callback=cb;
	cbcontext->user_data =user_data;

	gEmailcbList = g_slist_append(gEmailcbList,cbcontext);

	return EMAIL_ERROR_NONE;
}

int email_unset_message_sent_cb (email_h msg)
{

	int i,count;
	count = g_slist_length( gEmailcbList );
	GSList * node;
	email_cb_context *cbContext;

	if(msg ==NULL )
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : msg is null.", __FUNCTION__, EMAIL_ERROR_INVALID_PARAMETER);
		return EMAIL_ERROR_INVALID_PARAMETER;
		}
	email_s* msg_s = (email_s* )msg;
	for( i=0;i<count;i++)
	{
		node = g_slist_nth( gEmailcbList, i );
		if( node == NULL )
		break;
		
		
		cbContext= (email_cb_context *)node->data; 
		if(cbContext->handle == msg_s)
		{	
			gEmailcbList= g_slist_remove(gEmailcbList,node);
				break;
		}
		

			
	}
	return EMAIL_ERROR_NONE;


}

int _email_error_converter(int err)
{
	switch(err) 
	{
		

		case EMF_ERROR_INVALID_PARAM:
			return EMAIL_ERROR_INVALID_PARAMETER;

		case EMF_ERROR_DB_FAILURE:
			return EMAIL_ERROR_DB_FAILED;

		case EMF_ERROR_ACCOUNT_NOT_FOUND:
			return EMAIL_ERROR_ACCOUNT_NOT_FOUND;

		case EMF_ERROR_OUT_OF_MEMORY:
			return EMAIL_ERROR_OUT_OF_MEMORY;
			
		// Tizen email F/W  is often using this error type when it gets a null value from server
		//It could be caused from server or IPC.
		case EMF_ERROR_NULL_VALUE: 
			return EMAIL_ERROR_COMMUNICATION_WITH_SERVER_FAILED;

		case EMF_ERROR_IPC_SOCKET_FAILURE:
			return EMAIL_ERROR_COMMUNICATION_WITH_SERVER_FAILED;

		case EMF_ERROR_NONE:
			return EMAIL_ERROR_NONE;


		default:
			LOGE("[%s] OPERATION_FAILED(0x%08x) : Error from Email F/W. ret: (0x%08x) ", __FUNCTION__, EMAIL_ERROR_OPERATION_FAILED,err);
			return EMAIL_ERROR_OPERATION_FAILED;



	

	}
}



static void _monitorSendStatusCb(void* data, DBusMessage *message)
{
	DBusError error;
	email_cb_context *cbContext = NULL;
	if(dbus_message_is_signal(message, "User.Email.NetworkStatus", "email"))
	{
		dbus_error_init(&error);

		int status=0;
		int accountid=0;
		char* fileid=NULL;
		int mailid=0;
		int errorcode=0;
		if(dbus_message_get_args(message, &error, 
			DBUS_TYPE_INT32, &status, 
			DBUS_TYPE_INT32, &accountid, 
			DBUS_TYPE_STRING, &fileid, 
			DBUS_TYPE_INT32, &mailid,
			DBUS_TYPE_INT32, &errorcode,
			DBUS_TYPE_INVALID))
			{


				cbContext = _email_search_callback_by_emailid(mailid);
				if(cbContext == NULL)
					{
						LOGD_IF(DBG_MODE, "no callback matched!\n");
				}else{
					
						switch (status) {
							case NOTI_SEND_START:
								break;
								
							case NOTI_SEND_FAIL:

								 switch(errorcode)
					                     {
					                        case EMF_ERROR_NO_SIM_INSERTED:
					                        case EMF_ERROR_FLIGHT_MODE:
					                        case EMF_ERROR_SMTP_SEND_FAILURE:
					                        case EMF_ERROR_NO_SUCH_HOST:
					                        case EMF_ERROR_CONNECTION_FAILURE:
					                        case EMF_ERROR_CONNECTION_BROKEN:
					                        case EMF_ERROR_INVALID_SERVER:
					                        case EMF_ERROR_NO_RESPONSE:
					                            
					                            break;

					                        default:
												;
					                     }
								
								 cbContext->callback((email_h)cbContext->handle,(email_sending_e)EMAIL_SENDING_FAILED ,cbContext->user_data);
								break;

							case NOTI_SEND_FINISH:
								cbContext->callback((email_h)cbContext->handle,(email_sending_e)EMAIL_SENDING_SUCCEEDED ,cbContext->user_data);
								break;
					
						}
					}
			}
	}
}
void _email_add_dbus_filter(void)
{
	
    	E_DBus_Connection* connection=NULL;
    	DBusError error;
	E_DBus_Signal_Handler *handler = NULL;
	

	dbus_error_init(&error);

	connection = e_dbus_bus_get(DBUS_BUS_SYSTEM);

   	if (NULL == connection) {
 
    	}

	if(e_dbus_request_name(connection, "User.Email.NetworkStatus",
				     0,
				     NULL,
				     NULL) == NULL)
	{
		LOGD_IF(DBG_MODE, "Failed in e_dbus_request_name()\n");
	}

	 handler = e_dbus_signal_handler_add(connection,
			NULL,
			"/User/Email/NetworkStatus", 
			"User.Email.NetworkStatus", 
			"email",
			_monitorSendStatusCb, 
			NULL);

	if(handler == NULL)
	{
		LOGD_IF(DBG_MODE, "Failed in e_dbus_signal_handler_add()");
	}
}