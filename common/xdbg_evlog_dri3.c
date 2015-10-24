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
#include <X11/extensions/dri3proto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_dri3.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestDri3 (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_DRI3PixmapFromBuffer:
        {
            xDRI3PixmapFromBufferReq *stuff = (xDRI3PixmapFromBufferReq *)req;
            REPLY (": Pixmap(0x%x) drawable(0x%x) size(%u) width(%u) height(%u) stride(%u) depth(%u) bpp(%u)",
                (unsigned int)stuff->pixmap,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->size,
                (unsigned int)stuff->width,
                (unsigned int)stuff->height,
                (unsigned int)stuff->stride,
                (unsigned int)stuff->depth,
                (unsigned int)stuff->bpp);

            return reply;
        }

    case X_DRI3BufferFromPixmap:
        {
            xDRI3BufferFromPixmapReq *stuff = (xDRI3BufferFromPixmapReq *)req;
            REPLY (": Pixmap(0x%x)",
                (unsigned int)stuff->pixmap);

            return reply;
        }

    case X_DRI3FenceFromFD:
        {
            xDRI3FenceFromFDReq *stuff = (xDRI3FenceFromFDReq *)req;
            REPLY (": drawable(0x%x) fence(0x%x) initially_triggered(%s)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->fence,
                stuff->initially_triggered?"True":"False");

            return reply;
        }

    case X_DRI3FDFromFence:
        {
            xDRI3FDFromFenceReq *stuff = (xDRI3FDFromFenceReq *)req;
            REPLY (": drawable(0x%x) fence(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->fence);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventDri3 (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {

    default:
            break;
    }

    return reply;
}

static char *
_EvlogReplyDri3 (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_DRI3BufferFromPixmap:
        {
            if (evinfo->rep.isStart)
            {
                xDRI3BufferFromPixmapReply *stuff = (xDRI3BufferFromPixmapReply *)rep;
                REPLY (": nfd(%u) sequenceNumber(%u) size(%u) width(%u) height(%u) stride(%u) depth(%u) bpp(%u)",
                    (unsigned int)stuff->nfd,
                    (unsigned int)stuff->sequenceNumber,
                    (unsigned int)stuff->size,
                    (unsigned int)stuff->width,
                    (unsigned int)stuff->height,
                    (unsigned int)stuff->stride,
                    (unsigned int)stuff->depth,
                    (unsigned int)stuff->bpp);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_DRI3FDFromFence:
        {
            if (evinfo->rep.isStart)
            {
                xDRI3FDFromFenceReply *stuff = (xDRI3FDFromFenceReply *)rep;
                REPLY (": nfd(%u) sequenceNumber(%u)",
                    (unsigned int)stuff->nfd,
                    (unsigned int)stuff->sequenceNumber);
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
xDbgEvlogDri3GetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestDri3;
    extinfo->evt_func = _EvlogEventDri3;
    extinfo->rep_func = _EvlogReplyDri3;
#else
    ExtensionEntry *xext = CheckExtension (DRI3_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestDri3;
    extinfo->evt_func = _EvlogEventDri3;
    extinfo->rep_func = _EvlogReplyDri3;
#endif
}
