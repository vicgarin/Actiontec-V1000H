#ifndef __DBUSSEND_MSG_H__
#define __DBUSSEND_MSG_H__

#include <dbus/dbus.h>

typedef struct _dbussend_msg
{
    int data_length;    
    char buffer[];
}dbussend_msg_st, *p_dbussend_msg_st;

typedef struct _dbussend_cfg
{    
    DBusBusType type;
    int print_reply;
    int print_reply_literal;
    int reply_timeout;    
    int message_type;
    char *dest;
    char *path;
    char *address;    
}dbussend_cfg_st, *pdbussend_cfg_st;

typedef struct _dbussend_hdl
{
    DBusConnection *connection;
    DBusError error;
    DBusMessage *message;
    DBusMessageIter iter;

    dbussend_cfg_st dbussend_cfg;
}dbussend_hdl_st, *pdbussend_hdl_st;

dbussend_hdl_st *dbussend_init(void);
int dbussend_uninit(dbussend_hdl_st *p_dbussend_hdl);
int dbussend_sendmsg(dbussend_hdl_st *p_dbussend_hdl, const char *interface, const char *method, void *p_dbussend_msg, int len);
int dbussend_sendmsg_methodcall(dbussend_hdl_st *p_dbussend_hdl, const char *dest, const char *interface, const char *method, void *p_dbussend_msg, int len);

#endif

