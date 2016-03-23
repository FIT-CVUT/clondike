#ifndef MESSAGE_GENERIC_USER_MESSAGE_H
#define MESSAGE_GENERIC_USER_MESSAGE_H

int send_generic_user_message(struct nl_sock * sk, int index, int slot_index, int slot_type, int data_len, const char * data);

int prepare_generic_user_message(struct nl_msg ** ret_msg, int index, int slot_index, int slot_type, int data_len, const char * data);

#endif



