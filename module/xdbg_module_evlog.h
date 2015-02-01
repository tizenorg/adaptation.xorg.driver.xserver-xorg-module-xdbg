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

#ifndef __XDBG_MODULE_EVLOG_H__
#define __XDBG_MODULE_EVLOG_H__

#include "xdbg_module.h"

Bool    xDbgModuleEvlogInstallHooks   (XDbgModule *pMod);
void    xDbgModuleEvlogUninstallHooks (XDbgModule *pMod);

void    xDbgModuleEvlogPrintEvents (XDbgModule *pMod, Bool on, const char * client_name, char *reply, int *len);
void    xDbgModuleEvlogDetail (XDbgModule *pMod, int level, char *reply, int *len);
int     xDbgModuleEvlogInfoSetRule (XDbgModule *pMod, const int argc, const char ** argv, char *reply, int *len);

Bool    xDbgModuleEvlogSetEvlogPath  (XDbgModule *pMod, int pid, char *path, char *reply, int *len);

#endif /* __XDBG_MODULE_EVLOG_H__ */