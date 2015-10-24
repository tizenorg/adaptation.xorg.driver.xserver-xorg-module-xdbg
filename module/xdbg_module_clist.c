/**************************************************************************

xdbg

Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved

Contact: Boram Park <boram1288.park@samsung.com>
         Sangjin LEE <lsj119@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xdbg.h"
#include "xdbg_types.h"
#include "xdbg_module.h"
#include "xdbg_module_clist.h"

static ModuleTraceClientOption*
_findTraceClientOption (XDbgModule *pMod, char *client_name)
{
    int i = 0;

    for (i = 0 ; i < pMod->trace_cnt ; i++)
    {
        ModuleTraceClientOption *option = &pMod->trace_options[i];
        if (!strcmp(client_name, option->name))
        {
            return option;
        }
    }
    return NULL;
}

void
xDbgModuleCList (XDbgModule *pMod, char *reply, int *len)
{
    int i;

    XDBG_REPLY ("%6s   %6s   %s   %s\n", "INDEX", "PID", "BLOCKED", "NAME");

    for (i = 1; i < currentMaxClients && (0 < *len); i++)
    {
        ClientPtr pClient = clients[i];
        ModuleClientInfo *info;

        if (!pClient)
            continue;

        info = GetClientInfo (pClient);
        if (!info)
            continue;

        XDBG_REPLY ("%6d   %6d   %4d      %9s\n",
                        info->index, info->pid, pClient->ignoreCount, info->command);
    }
}


void
xDbgModuleClistTraceInit (XDbgModule *pMod)
{
    int i = 0;

    for (i = 1 ; i < currentMaxClients ; i++)
    {
        ClientPtr pClient = clients[i];
        ModuleClientInfo *info;

        if (!pClient)
            continue;

        info = GetClientInfo (pClient);
        xDbgModuleClistTraceAdd (pMod, info);
    }
}

void
xDbgModuleClistTraceAdd (XDbgModule *pMod, ModuleClientInfo *info)
{
    char *client_name = strrchr (info->command, '/');
    ModuleTraceClient *trace_client = NULL;
    ModuleTraceClient *client = NULL, *tmp = NULL;
    ModuleTraceClientOption *option = NULL;

    XDBG_DEBUG (MXDBG, "[TRACE_CLIENT] AddClist Trace command %s\n", info->command);

    if (client_name)
        client_name = client_name+1;
    else
        client_name = info->command;

    if (!(option = _findTraceClientOption (pMod, client_name)))
        return;

    xorg_list_for_each_entry_safe(client, tmp, &pMod->trace_list, link) {
        if (client->pid == info->pid)
        {
            XDBG_DEBUG (MXDBG, "[TRACE_CLIENT] Find Trace list:%s conn_cnt:%d+1\n", client_name, client->conn_cnt);
            client->conn_cnt++;
            return;
        }
    }

    trace_client = (ModuleTraceClient *) calloc (1, sizeof(ModuleTraceClient));
    XDBG_RETURN_IF_FAIL (trace_client != NULL);

    trace_client->pid = info->pid;
    trace_client->conn_cnt = 1;
    strncpy (trace_client->name, client_name, strlen(client_name)+1);

    XDBG_DEBUG (MXDBG, "[TRACE_CLIENT] Add Trace list:%s conn_cnt:%d\n", client_name, trace_client->conn_cnt);
    xorg_list_add (&trace_client->link, &pMod->trace_list);

    return;
}

void
xDbgModuleClistTraceRemove (XDbgModule *pMod, ModuleClientInfo *info)
{
    char *client_name = strrchr (info->command, '/');
    ModuleTraceClient *client = NULL, *tmp = NULL;
    ModuleTraceClientOption *option = NULL;

    XDBG_DEBUG (MXDBG, "[TRACE_CLIENT] RevmoeClist Trace command %s\n", info->command);

    if (client_name)
        client_name = client_name+1;
    else
        client_name = info->command;

    if (!(option = _findTraceClientOption (pMod, client_name)))
        return;

    xorg_list_for_each_entry_safe(client, tmp, &pMod->trace_list, link) {
        if (client->pid == info->pid)
        {
            if (client->conn_cnt == 1)
            {
                if (!strcmp(option->action, "cmd"))
                {
                    int ret = 0;
                    ErrorF ("[TRACE_CLIENT] Disconnect Trace client:%s pid:%d cmd:%s", client->name, client->pid, option->cmd);
                    if (strstr(option->cmd, "reboot"))
                        sleep (1);

                    ret = system (option->cmd);
                    ErrorF (" ret:%d\n", ret);
                }
                else if (!strcmp(option->action, "error"))
                {

                    FatalError ("[TRACE_CLIENT] %s\n", option->cmd);
                    ErrorF ("[TRACE_CLIENT] Disconnect Trace client:%s pid:%d msg:%s\n", client->name, client->pid, option->cmd);

                }
                else if (!strcmp(option->action, "log"))
                {
                    ErrorF ("[TRACE_CLIENT] Disconnect Trace client:%s pid:%d msg:%s\n", client->name, client->pid, option->cmd);
                }
                else
                {
                    XDBG_DEBUG (MXDBG, "[TRACE_CLIENT] Unkown action Trace client:%s pid:%d action:%s cmd:%s\n",
                                client->name, client->pid, option->action, option->cmd);
                }

                xorg_list_del (&client->link);
            }
            else
            {
                XDBG_DEBUG (MXDBG, "[TRACE_CLIENT] Unref client:%s conn_cnt:%d-1 \n", client_name, client->conn_cnt);
                client->conn_cnt--;
            }
            return;
        }
    }
    return;
}
