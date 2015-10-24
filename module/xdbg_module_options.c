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

#include <xdbg.h>

#include "xdbg_module.h"
#include "xdbg_module_command.h"
#include "xdbg_module_evlog.h"

/* Supported options */
typedef enum
{
    OPTION_DLOG,
    OPTION_LOG_PATH,
    OPTION_LOG_LEVEL,
    OPTION_EVLOG_PATH,
    OPTION_EVLOG_RULE_PATH,
    OPTION_XERROR_LOG_PATH,
    OPTION_TRACE_CNT,
} ModuleOption;

static const OptionInfoRec module_options[] =
{
    { OPTION_DLOG,             "dlog",             OPTV_BOOLEAN,  {0},    FALSE },
    { OPTION_LOG_PATH,         "log_path",         OPTV_STRING,   {0},    FALSE },
    { OPTION_LOG_LEVEL,        "log_level",        OPTV_INTEGER,  {0},    FALSE },
    { OPTION_EVLOG_PATH,       "evlog_path",       OPTV_STRING,   {0},    FALSE },
    { OPTION_EVLOG_RULE_PATH,  "evlog_rule_path",  OPTV_STRING,   {0},    FALSE },
    { OPTION_XERROR_LOG_PATH,  "xerror_log_path",  OPTV_STRING,   {0},    FALSE },
    { OPTION_TRACE_CNT,        "trace_cnt",        OPTV_INTEGER,  {0},    FALSE },
    { -1,                      NULL,               OPTV_NONE,     {0},    FALSE }
};

static void
xDbgModuleInitTraceClientOptions (XDbgModule *pMod, XF86OptionPtr pOpt)
{
    int i = 0;
    char name[128];
    char *option = NULL;

    pMod->trace_options = (ModuleTraceClientOption*) calloc (pMod->trace_cnt, sizeof(ModuleTraceClientOption));

    for (i = 0 ; i < pMod->trace_cnt ; i++)
    {
        snprintf (name, sizeof(name), "trace_client%d", i+1);

        option = xf86SetStrOption (pOpt, name, NULL);
        XDBG_INFO (MXDBG, "trace_client%d : %s\n", i+1, option);
        if (option)
        {
            char *token = NULL;
            char *tokp = NULL;

            token = strtok_r (option, ":", &tokp);
            if (token)
                strncpy (pMod->trace_options[i].name, token, strlen(token)+1);
            else
                XDBG_WARNING (MXDBG, "fail to parse trace client name:%s\n", option);

            token = strtok_r (NULL, ":", &tokp);
            if (token)
                strncpy (pMod->trace_options[i].action, token, strlen(token)+1);
            else
                XDBG_WARNING (MXDBG, "fail to parse trace client action:%s\n", option);

            token = strtok_r (NULL, ":", &tokp);
            if (token)
                strncpy (pMod->trace_options[i].cmd, token, strlen(token)+1);
            else
                XDBG_WARNING (MXDBG, "fail to parse trace client log or cmd:%s\n", option);

            free (option);
        }
    }
}

void
xDbgModuleParseOptions (XDbgModule *pMod, XF86OptionPtr pOpt)
{
    OptionInfoPtr options = xnfalloc (sizeof (module_options));
    const char *path;
    char temp[256];
    int log_level = XLOG_LEVEL_DEFAULT;

    memcpy (options, module_options, sizeof(module_options));

    xf86ProcessOptions (-1, pOpt, options);

    /* dlog */
    xf86GetOptValBool (options, OPTION_DLOG, &pMod->dlog);
    XDBG_INFO (MXDBG, "dlog: \"%s\"\n", (pMod->dlog)?"on":"off");
    xDbgLogEnableDlog (pMod->dlog);

    /* log_path */
    path = xf86GetOptValString (options, OPTION_LOG_PATH);
    XDBG_INFO (MXDBG, "log path: \"%s\"\n", (path)?path:"none");
    if (path)
    {
        snprintf (temp, sizeof(temp), "%s", path);
        xDbgModuleCommandInitLogPath (pMod, temp);
    }

    /* log_level */
    xf86GetOptValInteger (options, OPTION_LOG_LEVEL, &log_level);
    XDBG_INFO (MXDBG, "log default level: %d\n", log_level);
    xDbgLogSetLevel (XDBG_ALL_MODULE, log_level);

    /* evlog_path */
    path = xf86GetOptValString (options, OPTION_EVLOG_PATH);
    XDBG_INFO (MXDBG, "evlog path: \"%s\"\n", (path)?path:"none");
    if (path)
    {
        snprintf (temp, sizeof(temp), "%s", path);
        xDbgModuleEvlogSetEvlogPath (pMod, -1, temp, NULL, NULL);
    }

    /* evlog_rule_path */
    path = xf86GetOptValString (options, OPTION_EVLOG_RULE_PATH);
    XDBG_INFO (MXDBG, "evlog rule path: \"%s\"\n", (path)?path:"none");
    if (path)
    {
        snprintf (temp, sizeof(temp), "%s", path);
        xDbgModuleCommandInitEvlogRulePath (pMod, temp);
    }

    /* xerror_log_path */
    path = xf86GetOptValString (options, OPTION_XERROR_LOG_PATH);
    if (path)
    {
        pMod->xerror_log_path = strdup (path);
        xDbgModuleEvlogXErrorSetPath (pMod->xerror_log_path);
        XDBG_INFO (MXDBG, "evlog log path: \"%s\"\n", path);
    }
    else
        pMod->xerror_log_path = NULL;

    xf86GetOptValInteger (options, OPTION_TRACE_CNT, &pMod->trace_cnt);
    XDBG_INFO (MXDBG, "trace_cnt: %d\n", pMod->trace_cnt);

    xDbgModuleInitTraceClientOptions (pMod, pOpt);

    free (options);
}
