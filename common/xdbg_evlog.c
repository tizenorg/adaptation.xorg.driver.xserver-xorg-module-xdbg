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

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <X11/Xlib.h>

#include "xdbg_types.h"
#include "xdbg_evlog.h"
#include "bool_exp_rule_checker.h"
#include <X11/Xlibint.h>

#ifndef XDBG_CLIENT
#include "resource.h"
#include "region.h"
#include "dix.h"
#endif

static char *evt_type[] = { "Event", "Request", "Reply", "Flush", "Error" };
static char *evt_dir[]  = { "<====", "---->",   "<----", "*****", "<----"};

static RULE_CHECKER rc = NULL;

static void
_mergeArgs (char *target, int target_size, int argc, const char ** argv)
{
    int i;
    int len;

    for (i=0; i<argc; i++)
    {
        len = snprintf (target, target_size, "%s", argv[i]);
        target += len;
        target_size -= len;

        if (i != argc - 1)
        {
            *(target++) = ' ';
            target_size--;
        }
    }
}

static int
_strcasecmp(const char *str1, const char *str2)
{
    const u_char *us1 = (const u_char *) str1, *us2 = (const u_char *) str2;

    while (tolower(*us1) == tolower(*us2)) {
        if (*us1++ == '\0')
            return 0;
        us2++;
    }

    return (tolower(*us1) - tolower(*us2));
}

char*
xDbgEvlogGetCmd (char *path)
{
    char *p;
    if (!path)
        return NULL;
    p = strrchr (path, '/');
    return (p)?p+1:path;
}

Bool
xDbgEvlogRuleSet (const int argc, const char **argv, char *reply, int *len)
{
    const char * command;

    if (rc == NULL)
        rc = rulechecker_init();

    if (argc == 0)
    {
        rulechecker_print_rule (rc, reply, len);
        return TRUE;
    }

    command = argv[0];

    if (!_strcasecmp (command, "add"))
    {
        POLICY_TYPE policy_type;
        RC_RESULT_TYPE result;
        const char * policy = argv[1];
        char merge[8192]={0,}, rule[8192]={0,};
        int i, index = 0, size_rule;
        int apply = 0;

        if (argc < 3)
        {
            REPLY ("Error : Too few arguments.\n");
            return FALSE;
        }

        if (!_strcasecmp (policy, "ALLOW"))
            policy_type = ALLOW;
        else if (!_strcasecmp (policy, "DENY"))
            policy_type = DENY;
        else
        {
            REPLY ("Error : Unknown policy : [%s].\n          Policy should be ALLOW or DENY.\n", policy);
            return FALSE;
        }

        _mergeArgs (merge, sizeof (merge), argc - 2, &(argv[2]));

        size_rule = sizeof (rule) - 1;

        for (i = 0 ; i < strlen(merge) ; i++)
        {
            if(merge[i] == '\"' || merge[i] == '\'')
            {
                rule[index++] = ' ';
                if (index > size_rule)
                    return FALSE;

                continue;
            }

            if(merge[i] == '+')
            {
                rule[index++] = ' ';
                if (index > size_rule)
                    return FALSE;

                if (apply == 0)
                {
                    const char* plus = "|| type=reply || type=error";
                    int len = MIN (size_rule - index, strlen(plus));
                    strncat(rule, plus, len);
                    index += len;
                    if (index > size_rule)
                        return FALSE;

                    apply = 1;
                }
                continue;
            }
            rule[index++] = merge[i];
            if (index > size_rule)
                return FALSE;
        }

        result = rulechecker_add_rule (rc, policy_type, rule);
        if (result == RC_ERR_TOO_MANY_RULES)
        {
            REPLY ("Error : Too many rules were added.\n");
            return FALSE;
        }
        else if (result == RC_ERR_PARSE_ERROR)
        {
            REPLY ("Error : An error occured during parsing the rule [%s]\n", rule);
            return FALSE;
        }

        REPLY ("The rule was successfully added.\n\n");
        rulechecker_print_rule (rc, reply, len);
        return TRUE;
    }
    else if (!_strcasecmp (command, "remove"))
    {
        const char * remove_idx;
        int i;

        if (argc < 2)
        {
            REPLY ("Error : Too few arguments.\n");
            return FALSE;
        }

        for (i=0; i<argc - 1; i++)
        {
            remove_idx = argv[i+1];

            if (!_strcasecmp (remove_idx, "all"))
            {
                rulechecker_destroy (rc);
                rc = rulechecker_init();
                REPLY ("Every rules were successfully removed.\n");
            }
            else
            {
                int index = atoi (remove_idx);
                if (isdigit (*remove_idx) && rulechecker_remove_rule (rc, index) == 0)
                    REPLY ("The rule [%d] was successfully removed.\n", index);
                else
                    REPLY ("Rule remove fail : No such rule [%s].\n", remove_idx);
            }
        }
        rulechecker_print_rule (rc, reply, len);
        return TRUE;
    }
    else if (!_strcasecmp (command, "file"))
    {
        if (argc < 2)
        {
            REPLY ("Error : Too few arguments.\n");
            return FALSE;
        }

        if (!xDbgEvlogReadRuleFile(argv[1], reply, len))
            return FALSE;
        rulechecker_print_rule (rc, reply, len);
        
        return TRUE;
    }
    else if (!_strcasecmp (command, "print"))
    {
        rulechecker_print_rule (rc, reply, len);
        return TRUE;
    }
    else if (!_strcasecmp (command, "help"))
    {
        REPLY ("%s", rulechecker_print_usage());
        return TRUE;
    }

    REPLY ("%s\nUnknown command : [%s].\n\n", rulechecker_print_usage(), command);

    return TRUE;
}

Bool
xDbgEvlogRuleValidate (EvlogInfo *evinfo)
{
    const char *evlog_name = "";
    char *cmd = "";

    if (rc == NULL)
        rc = rulechecker_init ();

    if (!rc)
    {
        XDBG_LOG ("failed: create rulechecker\n");
        return FALSE;
    }

    cmd = xDbgEvlogGetCmd (evinfo->client.command);

    if (evinfo->type == REQUEST)
        evlog_name = evinfo->req.name;
    else if (evinfo->type == EVENT)
        evlog_name = evinfo->evt.name;
    else if (evinfo ->type == REPLY)
        evlog_name = evinfo->rep.name;

    return rulechecker_validate_rule (rc,
                                      evinfo->type,
                                      evinfo->req.id,
                                      evlog_name,
                                      evinfo->client.pid,
                                      cmd);
}

Bool
xDbgEvlogReadRuleFile(const char *filename, char *reply, int *len)
{
    int   fd = -1;
    char  fs[8096];
    char *pfs;
    int   rule_len;

    fd = open (filename, O_RDONLY);
    if (fd < 0)
    {
        REPLY ("failed: open '%s'. (%s)\n", filename, strerror(errno));
        return FALSE;
    }

    rule_len = read(fd, fs, sizeof(fs));
    pfs = fs;

    while (pfs - fs < rule_len)
    {
        int   new_argc = 3;
        char *new_argv[3] = {"add", };
        char  policy[64] = {0, };
        char  rule[1024] = {0, };
        int   i;

        if (pfs[0] == ' ' || pfs[0] == '\n')
        {
            pfs++;
            continue;
        }
        for (i = 0 ; pfs[i] != ' ' ; i++)
            policy[i] = pfs[i];

        new_argv[1] = policy;
        pfs += (strlen(new_argv[1]) + 1);

        memset(rule, 0, sizeof(rule));
        for (i = 0 ; pfs[i] != '\n' ; i++)
            rule[i] = pfs[i];

        new_argv[2] = rule;

        pfs += (strlen(new_argv[2]) + 1);


        if(!xDbgEvlogRuleSet ((const int) new_argc, (const char**) new_argv, reply, len))
        {
            return FALSE;
        }

    }

    if (fd >= 0)
        close (fd);

    return TRUE;
}


ExtensionInfo Evlog_extensions[] = {
    {xDbgEvlogCompositeGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogDamageGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogDri2GetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogGestureGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXinputGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogRandrGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXextDpmsGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXextShmGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXextSyncGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXextXtestGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXextXtestExt1GetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXextShapeGetBase, 0, 0, 0, NULL, NULL},
    {xDbgEvlogXvGetBase, 0, 0, 0, NULL, NULL}
};
ExtensionInfo* Sorted_Evlog_extensions;
int Extensions_size = 0;

static void
_ExtensionsSwap(ExtensionInfo* first, ExtensionInfo* second)
{
    ExtensionInfo temp;

    temp = *first ;
    *first = *second ;
    *second = temp ;
}

static Bool
_SortEvlogExtensions ()
{
    int i,j;
    int swap;

    Sorted_Evlog_extensions = (ExtensionInfo*)malloc(sizeof(Evlog_extensions));
    RETURN_VAL_IF_FAIL (Sorted_Evlog_extensions != NULL, FALSE);

    memcpy(Sorted_Evlog_extensions, Evlog_extensions, sizeof(Evlog_extensions));

    for (i = 0 ; i < Extensions_size - 1 ; i++)
    {
        swap = 0;
        for (j = 1 ; j < Extensions_size - i ; j++)
        {
            if(Sorted_Evlog_extensions[j-1].evt_base > Sorted_Evlog_extensions[j].evt_base)
            {
                _ExtensionsSwap(&Sorted_Evlog_extensions[j-1], &Sorted_Evlog_extensions[j]);
                swap = 1;
            }
        }
        if (!swap) break;
    }

    return TRUE;
}


Bool
xDbgEvlogGetExtensionEntry ()
{
    static int init = 0;
    static Bool success = FALSE;
    int i;

    if (init)
        return success;

    init = 1;
    Extensions_size = sizeof(Evlog_extensions) / sizeof (ExtensionInfo);

    for (i = 0 ; i < Extensions_size ; i++)
    {
        Evlog_extensions[i].get_base_func (Evlog_extensions + i);
    }

    if(!_SortEvlogExtensions ())
        return FALSE;


    success = TRUE;

    return success;
}


Bool
xDbgEvlogFillLog (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    static CARD32 prev;

    RETURN_VAL_IF_FAIL (evinfo->type >= 0 && (sizeof (evt_dir) / sizeof (char*)), FALSE);
    RETURN_VAL_IF_FAIL (evinfo->type >= 0 && (sizeof (evt_type) / sizeof (char*)), FALSE);

    if (evinfo->type == REPLY && !evinfo->rep.isStart)
    {
        if (detail_level >= EVLOG_PRINT_REPLY_DETAIL)
            REPLY ("%67s"," ");
        else
            return FALSE;
    }
    else
        REPLY ("[%10.3f][%5ld] %22s(%2d:%5d) %s %7s ",
                    evinfo->time / 1000.0,
                    evinfo->time - prev,
                    xDbgEvlogGetCmd (evinfo->client.command),
                    evinfo->client.index,
                    evinfo->client.pid,
                    evt_dir[evinfo->type],
                    evt_type[evinfo->type]);

    if (evinfo->type == REQUEST)
    {
        REPLY ("(");
        reply = xDbgEvlogReqeust (evinfo, detail_level, reply, len);
        REPLY (")");
    }
    else if (evinfo->type == EVENT)
    {
        evinfo->evt.size = sizeof (xEvent);
        REPLY ("(");
        reply = xDbgEvlogEvent (evinfo, detail_level, reply, len);
        REPLY (")");
    }
    else if (evinfo->type == REPLY)
    {
        REPLY ("(");
        reply = xDbgEvlogReply (evinfo, detail_level, reply, len);
        REPLY (")");
    }
    else if (evinfo->type == ERROR)
    {
        REPLY("(ErrorCode(0x%02x) resourceID(0x%lx) majorCode(%d) minorCode(%d))",
            evinfo->err.errorCode,
            evinfo->err.resourceID,
            evinfo->err.majorCode,
            evinfo->err.minorCode);
    }
    else
    {
        const char *evlog_name = "";
        if (evinfo->type == REQUEST)
            evlog_name = evinfo->req.name;
        else if (evinfo->type == EVENT)
            evlog_name = evinfo->evt.name;
        else if (evinfo->type == REPLY)
            evlog_name = evinfo->rep.name;
        REPLY ("(%s)", evlog_name);
    }

    REPLY ("\n");

    prev = evinfo->time;

    return TRUE;
}


void xDbgDistroyAtomList (EvlogInfo *evinfo)
{
    EvlogAtomTable *cur = NULL, *next = NULL;

    if (!evinfo->evatom.init)
        return;

    xorg_list_for_each_entry_safe(cur, next, &evinfo->evatom.list, link)
    {
        xorg_list_del(&cur->link);
        free (cur);
        cur = NULL;
    }
    evinfo->evatom.init = 0;
    evinfo->evatom.size = 0;
}

void xDbgDistroyRegionList (EvlogInfo *evinfo)
{
    EvlogRegionTable *cur = NULL, *next = NULL;

    if (!evinfo->evregion.init)
        return;

    xorg_list_for_each_entry_safe(cur, next, &evinfo->evregion.list, link)
    {
        xorg_list_del(&cur->link);
        free (cur);
        cur = NULL;
    }
    evinfo->evregion.init = 0;
    evinfo->evregion.size = 0;
}

char* xDbgGetAtom(Atom atom, EvlogInfo *evinfo, char *reply, int *len)
{
    EvlogAtomTable *table;
#ifndef XDBG_CLIENT
    table = malloc (sizeof(EvlogAtomTable));
    if (!table)
        return reply;

    evinfo->mask |= EVLOG_MASK_ATOM;
    table->xid = atom;

    if (!evinfo->evatom.init)
    {
        xorg_list_init(&evinfo->evatom.list);
        evinfo->evatom.init = 1;
    }

    if (NameForAtom(atom))
        snprintf (table->buf, XDBG_BUF_SIZE, "%s", (char*)NameForAtom(atom));
    else
        snprintf (table->buf, XDBG_BUF_SIZE, "0x%lx", atom);

    xorg_list_add(&table->link, &evinfo->evatom.list);
    evinfo->evatom.size++;
#endif
    xorg_list_for_each_entry(table, &evinfo->evatom.list, link)
        if(table->xid == atom)
        {
            REPLY ("(%s)", table->buf);
            break;
        }

    return reply;
}

char* xDbgGetRegion(XserverRegion region, EvlogInfo *evinfo, char *reply, int *len)
{
    EvlogRegionTable *table;
#ifndef XDBG_CLIENT
    extern _X_EXPORT RESTYPE RegionResType;
    RegionPtr pRegion;
    BoxPtr rects;
    int nrect, i;
    int s;
    int err = dixLookupResourceByType((pointer *) &pRegion, region,
                                       RegionResType, (ClientPtr)evinfo->client.pClient,
                                       DixReadAccess);

    evinfo->mask |= EVLOG_MASK_REGION;

    if (!evinfo->evregion.init)
    {
        xorg_list_init(&evinfo->evregion.list);
        evinfo->evregion.init = 1;
    }

	if (err != Success)
	{
        table = malloc (sizeof(EvlogAtomTable));
        if (!table)
            return reply;

        table->xid = region;

        snprintf (table->buf, XDBG_BUF_SIZE, "0x%lx", region);
        xorg_list_add(&table->link, &evinfo->evregion.list);
        evinfo->evregion.size++;
    }
    else
    {
        nrect = RegionNumRects(pRegion);
        rects = RegionRects(pRegion);

        for (i = 0; i < nrect; i++)
        {
            table = malloc (sizeof(EvlogAtomTable));
            if (!table)
                return reply;

            table->xid = region;

            s = 0;
            s += snprintf (table->buf + s, XDBG_BUF_SIZE - s,
                           "[%d,%d %dx%d]",
                               rects[i].x1,
                               rects[i].y1,
                               rects[i].x2 - rects[i].x1,
                               rects[i].y2 - rects[i].y1);
            xorg_list_add(&table->link, &evinfo->evregion.list);
            evinfo->evregion.size++;
        }
    }

#endif
    REPLY("(");
    xorg_list_for_each_entry(table, &evinfo->evregion.list, link)
        if(table->xid == region)
        {
            REPLY ("%s", table->buf);
            if(table != xorg_list_last_entry(&evinfo->evregion.list, EvlogRegionTable, link))
                REPLY (", ");
        }
    REPLY(")");

    return reply;
}
