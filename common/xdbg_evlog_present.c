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
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>

#include <dix.h>
#define XREGISTRY
#include <registry.h>
#include <xace.h>
#include <xacestr.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <windowstr.h>
#include <X11/extensions/presentproto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_present.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestPresent (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_PresentPixmap:
        {
            xPresentPixmapReq *stuff = (xPresentPixmapReq *)req;
            REPLY (": window(0x%x) pixmap(0x%x) (s,v,u)(%u,0x%x,0x%x) x,y(%d,%d) crtc(%u) wait(0x%x) idle(0x%x) op(0x%x) (t,d,r)(%u,%u,%u)",
                (unsigned int)stuff->window,
                (unsigned int)stuff->pixmap,
                (unsigned int)stuff->serial,
                (unsigned int)stuff->valid,
                (unsigned int)stuff->update,
                (int)stuff->x_off,
                (int)stuff->y_off,
                (unsigned int)stuff->target_crtc,
                (unsigned int)stuff->wait_fence,
                (unsigned int)stuff->idle_fence,
                (unsigned int)stuff->options,
                (unsigned int)stuff->target_msc,
                (unsigned int)stuff->divisor,
                (unsigned int)stuff->remainder);

            return reply;
        }

    case X_PresentNotifyMSC:
        {
            xPresentNotifyMSCReq *stuff = (xPresentNotifyMSCReq *)req;
            REPLY (": window(0x%x) serial(%u) target_msc(%lu) divisor(%lu) remainder(%lu)",
                (unsigned int)stuff->window,
                (unsigned int)stuff->serial,
                (unsigned long)stuff->target_msc,
                (unsigned long)stuff->divisor,
                (unsigned long)stuff->remainder);

            return reply;
        }

    case X_PresentSelectInput :
        {
            xPresentSelectInputReq *stuff = (xPresentSelectInputReq *)req;
            REPLY (": eid(0x%x) window(0x%x) eventMask (0x%x)",
                (unsigned int)stuff->eid,
                (unsigned int)stuff->window,
                (unsigned int)stuff->eventMask);

            return reply;
        }

    case X_PresentQueryCapabilities :
        {
            xPresentQueryCapabilitiesReq *stuff = (xPresentQueryCapabilitiesReq *)req;
            REPLY (": target(0x%x)",
                (unsigned int)stuff->target);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventPresent (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {

    case PresentConfigureNotify:
         {
             xPresentConfigureNotify *stuff = (xPresentConfigureNotify *) evt;
             REPLY (": window(0x%x)(%ux%u+%d+%d) off x,y (%d,%d)  pximap width(%u) height(%u) flags(0x%x)",
                 (unsigned int)stuff->window,
                 (unsigned int)stuff->width,
                 (unsigned int)stuff->height,
                 (int)stuff->x,
                 (int)stuff->y,
                 (int)stuff->off_x,
                 (int)stuff->off_y,
                 (unsigned int)stuff->pixmap_width,
                 (unsigned int)stuff->pixmap_height,
                 (unsigned int)stuff->pixmap_flags);
    
             evinfo->evt.size = sizeof (xPresentConfigureNotify);
    
             return reply;
         }
    
     case PresentCompleteNotify:
         {
             xPresentCompleteNotify *stuff = (xPresentCompleteNotify *) evt;
             REPLY (": window(0x%x) serial(0x%x) kind(%u) mode(%u) ust(%lu)",
                 (unsigned int)stuff->window,
                 (unsigned int)stuff->serial,
                 (unsigned int)stuff->kind,
                 (unsigned int)stuff->mode,
                 (unsigned long)stuff->ust);
    
            evinfo->evt.size = sizeof (xPresentCompleteNotify);

             return reply;
         }
    
    case PresentIdleNotify:
        {
            xPresentIdleNotify *stuff = (xPresentIdleNotify *) evt;
            REPLY (": window(0x%x) serial(0x%x) pixmap(0x%x) idle_fence(0x%x)",
                (unsigned int)stuff->window,
                (unsigned int)stuff->serial,
                (unsigned int)stuff->pixmap,
                (unsigned int)stuff->idle_fence);
    
            evinfo->evt.size = sizeof (xPresentIdleNotify);

            return reply;
        }
#if 0
    case PresentRedirectNotify:
        {

            xPresentRedirectNotify *stuff = (xPresentRedirectNotify *) evt;
            REPLY (": window(0x%x) pixmap(0x%x) serial(%u) valid(0x%x) update(0x%x) x,y_off(%d,%d) target_crtc(%u) wait_fence(0x%x) idle_fence(0x%x) options(0x%x) target_msc(%u) divisor(%u) remainder(%u)",
                (unsigned int)stuff->pixmap,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->serial,
                (unsigned int)stuff->valid_region,
                (unsigned int)stuff->update_region,
                (int)stuff->x_off,
                (int)stuff->y_off,
                (unsigned int)stuff->target_crtc,
                (unsigned int)stuff->wait_fence,
                (unsigned int)stuff->idle_fence,
                (unsigned int)stuff->options,
                (unsigned int)stuff->target_msc,
                (unsigned int)stuff->divisor,
                (unsigned int)stuff->remainder);

            evinfo->evt.size = sizeof (xPresentRedirectNotify);

            return reply;
        }
#endif

    default:
            break;
    }

    return reply;
}

static char *
_EvlogReplyPresent (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_PresentQueryCapabilities:
        {
            if (evinfo->rep.isStart)
            {
                xPresentQueryCapabilitiesReply *stuff = (xPresentQueryCapabilitiesReply *)rep;
                REPLY (": capabilities(0x%x)",
                    (unsigned int)stuff->capabilities);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

void
xDbgEvlogPresentGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestPresent;
    extinfo->evt_func = _EvlogEventPresent;
    extinfo->rep_func = _EvlogReplyPresent;
#else
    ExtensionEntry *xext = CheckExtension (PRESENT_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestPresent;
    extinfo->evt_func = _EvlogEventPresent;
    extinfo->rep_func = _EvlogReplyPresent;
#endif
}
