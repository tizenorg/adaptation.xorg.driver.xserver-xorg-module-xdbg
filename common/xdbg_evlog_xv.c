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

#include <X11/extensions/Xvlib.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvproto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_xv.h"

static char *
_EvlogRequestXv(xReq *req, char *reply, int *len)
{
    xReq *stuff = req;

    switch (stuff->data)
    {
    case xv_GrabPort:
        {
            xvGrabPortReq *stuff = (xvGrabPortReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->port);

            return reply;
        }

    case xv_UngrabPort:
        {
            xvUngrabPortReq *stuff = (xvUngrabPortReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->port);

            return reply;
        }

    case xv_GetStill:
        {
            xvGetStillReq *stuff = (xvGetStillReq *)req;
            REPLY (": XID(0x%lx) Drawable(0x%lx) GC(0x%lx) Vid(%d,%d) Drw(%d,%d)",
                stuff->port,
                stuff->drawable,
                stuff->gc,
                stuff->vid_x,
                stuff->vid_y,
                stuff->drw_x,
                stuff->drw_y);

            return reply;
        }

    case xv_StopVideo:
        {
            xvStopVideoReq *stuff = (xvStopVideoReq *)req;
            REPLY (": XID(0x%lx) Drawable(0x%lx)",
                stuff->port,
                stuff->drawable);

            return reply;
        }

    case xv_SelectVideoNotify:
        {
            xvSelectVideoNotifyReq *stuff = (xvSelectVideoNotifyReq *)req;
            REPLY (": XID(0x%lx) On/Off(%d)",
                stuff->drawable,
                stuff->onoff);

            return reply;
        }

    case xv_SelectPortNotify:
        {
            xvSelectPortNotifyReq *stuff = (xvSelectPortNotifyReq *)req;
            REPLY (": XID(0x%lx) On/Off(%d)",
                stuff->port,
                stuff->onoff);

            return reply;
        }

    case xv_QueryBestSize:
        {
            xvQueryBestSizeReq *stuff = (xvQueryBestSizeReq *)req;
            REPLY (": XID(0x%lx) VidSize(%dx%d) DrwSize(%dx%d)",
                stuff->port,
                stuff->vid_w,
                stuff->vid_h,
                stuff->drw_w,
                stuff->drw_h);

            return reply;
        }

    case xv_SetPortAttribute:
        {
            xvSetPortAttributeReq *stuff = (xvSetPortAttributeReq *)req;
            REPLY (": XID(0x%lx) Attribute(0x%lx) value(0x%lx)",
                stuff->port,
                stuff->attribute,
                stuff->value);

            return reply;
        }

    case xv_PutImage:
        {
            xvPutImageReq *stuff = (xvPutImageReq *)req;
            REPLY (": XID(0x%lx) Drawable(0x%lx) GC(0x%lx) ID(0x%lx) src(%d,%d) drw(%d,%d)",
                stuff->port,
                stuff->drawable,
                stuff->gc,
                stuff->id,
                stuff->src_x,
                stuff->src_y,
                stuff->drw_x,
                stuff->drw_y);

            return reply;
        }

    case xv_ShmPutImage:
        {
            xvShmPutImageReq *stuff = (xvShmPutImageReq *)req;
            REPLY (": XID(0x%lx) Drawable(0x%lx) GC(0x%lx) ID(0x%lx) src(%d,%d) drw(%d,%d)",
                stuff->port,
                stuff->drawable,
                stuff->gc,
                stuff->id,
                stuff->src_x,
                stuff->src_y,
                stuff->drw_x,
                stuff->drw_y);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogEventXv (xEvent *evt, int first_base, char *reply, int *len)
{
    xEvent *stuff = evt;

    switch ((stuff->u.u.type & 0x7F) - first_base)
    {
    case XvVideoNotify:
        {
            XvVideoNotifyEvent *stuff = (XvVideoNotifyEvent *) evt;
            REPLY (": XID(0x%lx) reason(%ld) portID(0x%lx)",
                stuff->drawable,
                stuff->reason,
                stuff->port_id);

            return reply;
        }

    case XvPortNotify:
        {
            XvPortNotifyEvent *stuff = (XvPortNotifyEvent *) evt;
            REPLY (": XID(0x%lx) Attribute(%lx) Value(%ld)",
                stuff->port_id,
                stuff->attribute,
                stuff->value);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

void
xDbgEvlogXvGetBase (void *dpy, ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    Display *d = (Display*)dpy;

    RETURN_IF_FAIL (d != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    if (!XQueryExtension(d, XvName, &extinfo->opcode, &extinfo->evt_base, &extinfo->err_base))
    {
        fprintf (stderr, "[UTILX] no Xv extension. \n");
        return;
    }
    extinfo->req_func = _EvlogRequestXv;
    extinfo->evt_func = _EvlogEventXv;
#else
    ExtensionEntry *xext = CheckExtension (XvName);
    RETURN_IF_FAIL (xext != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXv;
    extinfo->evt_func = _EvlogEventXv;

#endif
}