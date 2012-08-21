#include <tet_api.h>
#include <messaging/email.h>
#include <stdlib.h>

static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_messaging_email_create_message_p(void);
//static void utc_messaging_email_create_message_n(void);
static void utc_messaging_email_destroy_message_p(void);
static void utc_messaging_email_destroy_message_n(void);
static void utc_messaging_email_set_subject_p(void);
static void utc_messaging_email_set_subject_n(void);
static void utc_messaging_email_set_body_p(void);
static void utc_messaging_email_set_body_n(void);
//static void utc_messaging_email_get_body_p(void);
//static void utc_messaging_email_get_body_n(void);
static void utc_messaging_email_add_recipient_p(void);
static void utc_messaging_email_add_recipient_n(void);
static void utc_messaging_email_remove_all_recipients_p(void);
static void utc_messaging_email_remove_all_recipients_n(void);
static void utc_messaging_email_add_attach_p(void);
static void utc_messaging_email_add_attach_n(void);
static void utc_messaging_email_remove_all_attachment_p(void);
static void utc_messaging_email_remove_all_attachment_n(void);
static void utc_messaging_email_send_message_p(void);
static void utc_messaging_email_send_message_n(void);
static void utc_messaging_email_set_message_sent_cb(void);
static void utc_messaging_email_set_message_sent_cb_n(void);
static void utc_messaging_email_unset_message_sent_cb(void);
static void utc_messaging_email_unset_message_sent_cb_n(void);



enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

struct tet_testlist tet_testlist[] = {

	{ utc_messaging_email_create_message_p, POSITIVE_TC_IDX },
	//{ utc_messaging_email_create_message_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_destroy_message_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_destroy_message_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_set_subject_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_set_subject_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_set_body_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_set_body_n, NEGATIVE_TC_IDX },
	//{ utc_messaging_email_get_body_p, POSITIVE_TC_IDX },
	//{ utc_messaging_email_get_body_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_add_recipient_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_add_recipient_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_remove_all_recipients_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_remove_all_recipients_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_add_attach_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_add_attach_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_remove_all_attachment_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_remove_all_attachment_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_send_message_p, POSITIVE_TC_IDX },
	{ utc_messaging_email_send_message_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_set_message_sent_cb, POSITIVE_TC_IDX },
	{ utc_messaging_email_set_message_sent_cb_n, NEGATIVE_TC_IDX },
	{ utc_messaging_email_unset_message_sent_cb, POSITIVE_TC_IDX },
	{ utc_messaging_email_unset_message_sent_cb_n, NEGATIVE_TC_IDX },


	{ NULL, 0 },
};


static void startup(void)
{
	/* start of TC */
	tet_printf("\n TC start");
}


static void cleanup(void)
{
	/* end of TC */
	tet_printf("\n TC end");
}


/**
 * @brief Positive test case of utc_messaging_email_create_message_p()
 */
static void utc_messaging_email_create_message_p(void)
{
       int ret = EMAIL_ERROR_NONE;

	email_h msg;
        //  Invalid parameter test
	ret = email_create_message(&msg);

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_create_message");
	}
	else {
        dts_message("email_create_message", "email_create_message ret : %d", ret);
        dts_fail("email_create_message");
	}
}


/**
 * @brief Positive test case of utc_messaging_email_destroy_message_p()
 */
static void utc_messaging_email_destroy_message_p(void)
{
       int ret = EMAIL_ERROR_NONE;

	email_h msg;
        //  Invalid parameter test
	ret = email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}


	ret=email_destroy_message(msg);
	
	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_destroy_message");
	}
	else {
        dts_message("email_destroy_message", "email_destroy_message ret : %d", ret);
        dts_fail("email_destroy_message");
	}
}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_destroy_message_n(void)
{
 	

	int ret = EMAIL_ERROR_NONE;


	ret=email_destroy_message(NULL);
	
	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_destroy_message");
	}
	else {
        dts_message("email_destroy_message", "email_destroy_message ret : %d", ret);
        dts_fail("email_destroy_message");
	}
	
}

/**
 * @brief Positive test case of telephony_get_cell_id()
 */
static void utc_messaging_email_set_subject_p(void)
{
       int ret = EMAIL_ERROR_NONE;

	email_h msg;
        //  Invalid parameter test
	ret = email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}

	ret=email_set_subject(msg,"titel: First email!!!");


	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_set_subject");
	}
	else {
        dts_message("email_set_subject", "email_set_subject ret : %d", ret);
        dts_fail("email_set_subject");
	}

}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_set_subject_n(void)
{
 	

	int ret = EMAIL_ERROR_NONE;

	ret=email_set_subject(NULL,"titel: First email!!!");


	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_set_subject");
	}
	else {
        dts_message("email_set_subject", "email_set_subject ret : %d", ret);
        dts_fail("email_set_subject");
	}
	
}

/**
 * @brief Positive test case of telephony_get_cell_id()
 */
static void utc_messaging_email_set_body_p(void)
{
       int ret = EMAIL_ERROR_NONE;

	email_h msg;
        //  Invalid parameter test
	ret = email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}

	ret=email_set_body(msg,"First SMS message!!!");


	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_set_body");
	}
	else {
        dts_message("email_set_body", "email_set_body ret : %d", ret);
        dts_fail("email_set_body");
	}

}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_set_body_n(void)
{
 	

	int ret = EMAIL_ERROR_NONE;

	ret=email_set_body(NULL,"First SMS message!!!");


	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_set_body");
	}
	else {
        dts_message("email_set_body", "email_set_body ret : %d", ret);
        dts_fail("email_set_body");
	}
	
}
#if 0
/**
 * @brief Positive test case of telephony_get_cell_id()
 */
static void utc_messaging_email_get_body_p(void)
{
       int ret = EMAIL_ERROR_NONE;
	email_h msg;
	const char *str;
	
	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
		
	ret=email_set_body(msg,"First SMS message!!!");

	if(ret != EMAIL_ERROR_NONE)  
	{
       	dts_message("email_set_body", "email_set_body ret : %d", ret);
        	dts_fail("email_set_body");
	}

	ret=email_get_body(msg,str);


	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_get_body");
	}
	else {
        dts_message("email_get_body", "email_get_body ret : %d", ret);
        dts_fail("email_get_body");
	}

	
}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_get_body_n(void)
{
 	

	int ret = EMAIL_ERROR_NONE;
	const char *str;
	ret=email_get_body(NULL,str);

	
	
	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_get_body");
	}
	else {
        dts_message("email_get_body", "email_get_body ret : %d", ret);
        dts_fail("email_get_body");
	}
	
}

#endif

/**
 * @brief Positive test case of telephony_get_cell_id()
 */
static void utc_messaging_email_add_recipient_p(void)
{

	 int ret = EMAIL_ERROR_NONE;
	email_h msg;
	

	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	ret=email_add_recipient(msg,EMAIL_RECIPIENT_TYPE_TO,"qqaappp@gmail.com");

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_add_recipien");
	}
	else {
        dts_message("email_add_recipien", "email_add_recipien ret : %d", ret);
        dts_fail("email_add_recipien");
	}
}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_add_recipient_n(void)
{
 	
	email_h msg;
	int ret = EMAIL_ERROR_NONE;
	ret=email_add_recipient(msg,EMAIL_RECIPIENT_TYPE_TO,"qqaappp@gmail.com");

	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_add_recipien");
	}
	else {
        dts_message("email_add_recipien", "email_add_recipien ret : %d", ret);
        dts_fail("email_add_recipien");
	}

	
}

/**
 * @brief Positive test case of telephony_get_cell_id()
 */
 //TODO: need to do multiple test
static void utc_messaging_email_remove_all_recipients_p(void)
{
	 int ret = EMAIL_ERROR_NONE;

	email_h msg;
	

	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	ret=email_add_recipient(msg,EMAIL_RECIPIENT_TYPE_TO,"qqaappp@gmail.com");

	if(ret != EMAIL_ERROR_NONE)
	{
        	dts_message("email_add_recipien", "email_add_recipien ret : %d", ret);
        	dts_fail("email_add_recipien");
	}

	ret=email_remove_all_recipients(msg);

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_remove_all_recipients");
	}
	else {
        dts_message("email_remove_all_recipients", "email_remove_all_recipients ret : %d", ret);
        dts_fail("email_remove_all_recipients");
	}

}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_remove_all_recipients_n(void)
{
 	

	int ret = EMAIL_ERROR_NONE;

	ret=email_remove_all_recipients(NULL);

	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_remove_all_recipients");
	}
	else {
        dts_message("email_remove_all_recipients", "email_remove_all_recipients ret : %d", ret);
        dts_fail("email_remove_all_recipients");
	}

	
}


/**
 * @brief Positive test case of telephony_get_cell_id()
 */
 //TODO: need to do multiple test
static void utc_messaging_email_add_attach_p(void)
{
	 int ret = EMAIL_ERROR_NONE;

	email_h msg;
	
	FILE* file = NULL;


       file= fopen("/tmp/emaildtstest_.txt", "w");
	if(file ==NULL)
	{
		dts_message("email_add_attach", "temporary file for test(/tmp/emaildtstest_.txt) is not created");
        	dts_fail("email_add_attach");
		}
	else
	{
		 fclose(file);
		}
	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	
	ret=email_add_attach(msg,"/tmp/emaildtstest_.txt");

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_add_attach");
	}
	else {
        dts_message("email_add_attach", "email_add_attach ret : %d", ret);
        dts_fail("email_add_attach");
	}

}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_add_attach_n(void)
{
 	

	int ret = EMAIL_ERROR_NONE;

	ret=email_add_attach(NULL,NULL);

	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_add_attach");
	}
	else {
        dts_message("email_add_attach", "email_add_attach ret : %d", ret);
        dts_fail("email_add_attach");
	}

	
}

/**
 * @brief Positive test case of telephony_get_cell_id()
 */
 //TODO: need to do multiple test
static void utc_messaging_email_remove_all_attachment_p(void)
{
	 int ret = EMAIL_ERROR_NONE;

	email_h msg;
	
	FILE* file = NULL;


       file= fopen("/tmp/emaildtstest_.txt", "w");
	if(file ==NULL)
	{
		dts_message("email_add_attach", "temporary file for test(/tmp/emaildtstest_.txt) is not created");
        	dts_fail("email_add_attach");
		}
	else
	{
		 fclose(file);
		}
	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	
	ret=email_add_attach(msg,"/tmp/emaildtstest_.txt");

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_add_attach", "email_add_attach ret : %d", ret);
        	dts_fail("email_add_attach");
		}

	ret=email_remove_all_attachments(msg);

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_remove_all_attachment");
	}
	else {
        dts_message("email_remove_all_attachment", "email_remove_all_attachment ret : %d", ret);
        dts_fail("email_remove_all_attachment");
	}

}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_remove_all_attachment_n(void)
{
 	
	email_h msg;
	int ret = EMAIL_ERROR_NONE;

	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	
	ret=email_remove_all_attachments(msg);

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_remove_all_attachment");
	}
	else {
        dts_message("email_remove_all_attachment", "email_remove_all_attachment ret : %d", ret);
        dts_fail("email_remove_all_attachment");
	}

	
}
/**
 * @brief Positive test case of telephony_get_cell_id()
 */
static void utc_messaging_email_send_message_p(void)
{
	int ret = EMAIL_ERROR_NONE;
	email_h msg;
	

	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	ret=email_add_recipient(msg,EMAIL_RECIPIENT_TYPE_TO,"qqaappp@gmail.com");

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_add_recipient", "email_add_recipient ret : %d", ret);
        	dts_fail("email_add_recipient");
		}

	ret=email_set_body(msg,"First SMS message!!!");


	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_set_body");
	}
	else {
        dts_message("email_set_body", "email_set_body ret : %d", ret);
        dts_fail("email_set_body");
	}

		
	ret=email_send_message(msg, false);

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_send_message");
	}
	else {
        dts_message("email_send_message", "email_send_message ret : %d", ret);
        dts_fail("email_send_message");
	}

}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
 //TODO: add the case where  one of what shold be set is missing
 // for example, recipient ,attachment or body is not set. 
static void utc_messaging_email_send_message_n(void)
{
	int ret = EMAIL_ERROR_NONE;

	ret=email_send_message(NULL, false);

	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_send_message");
	}
	else {
        dts_message("email_send_message", "email_send_message ret : %d", ret);
        dts_fail("email_send_message");
	}
	
}

/**
 * @brief Positive test case of telephony_get_cell_id()
 */

void email_cb(email_h handle, email_sending_e result, void *user_data)
{
    	printf("CALLBACK EXECUTED\n");
	printf("transport status  = %d\n", result);
     
  
}
static void utc_messaging_email_set_message_sent_cb(void)
{
       int ret = EMAIL_ERROR_NONE;
	email_h msg;


	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	
	ret=email_set_message_sent_cb(msg,email_cb,NULL);

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_set_message_sent_cb");
	}
	else {
        dts_message("email_set_message_sent_cb", "email_set_message_sent_cb ret : %d", ret);
        dts_fail("email_set_message_sent_cb");
	}
}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_set_message_sent_cb_n(void)
{


	int ret = EMAIL_ERROR_NONE;

	ret=email_set_message_sent_cb(NULL,NULL,NULL);

	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_set_message_sent_cb");
	}
	else {
        dts_message("email_set_message_sent_cb", "email_set_message_sent_cb ret : %d", ret);
        dts_fail("email_set_message_sent_cb");
	}
	
}

/**
 * @brief Positive test case of telephony_get_cell_id()
 */
static void utc_messaging_email_unset_message_sent_cb(void)
{
        int ret = EMAIL_ERROR_NONE;
	email_h msg;


	ret =email_create_message(&msg);

	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_create_message", "email_create_message ret : %d", ret);
        	dts_fail("email_create_message");
		}
	
	ret=email_set_message_sent_cb(msg,email_cb,NULL);
	if(ret != EMAIL_ERROR_NONE) 
	{
		dts_message("email_set_message_sent_cb", "email_set_message_sent_cb ret : %d", ret);
        	dts_fail("email_set_message_sent_cb");
		}

	ret=email_unset_message_sent_cb(msg );

	if(ret == EMAIL_ERROR_NONE) {
	    dts_pass("email_set_message_sent_cb");
	}
	else {
        dts_message("email_unset_message_sent_cb", "email_unset_message_sent_cb ret : %d", ret);
        dts_fail("email_unset_message_sent_cb");
	}
}


/**
 * @brief Negative test case of telephony_get_cell_id()
 */
static void utc_messaging_email_unset_message_sent_cb_n(void)
{


	int ret = EMAIL_ERROR_NONE;
	ret=email_unset_message_sent_cb(NULL );

	if(ret == EMAIL_ERROR_INVALID_PARAMETER) {
	    dts_pass("email_unset_message_sent_cb");
	}
	else {
        dts_message("email_unset_message_sent_cb", "email_unset_message_sent_cb ret : %d", ret);
        dts_fail("email_unset_message_sent_cb");
	}

	
}

