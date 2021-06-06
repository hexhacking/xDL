// Copyright (c) 2020-present, HexHacking Team. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Created by caikelun on 2021-02-21.

#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <dlfcn.h>
#include "xdl_linker.h"
#include "xdl.h"
#include "xdl_util.h"

#ifndef __LP64__
#define XDL_LINKER_BASENAME "linker"
#else
#define XDL_LINKER_BASENAME "linker64"
#endif

#define XDL_LINKER_SYM_MUTEX         "__dl__ZL10g_dl_mutex"
#define XDL_LINKER_SYM_DLOPEN_EXT_N  "__dl__ZL10dlopen_extPKciPK17android_dlextinfoPv"
#define XDL_LINKER_SYM_DO_DLOPEN_N   "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv"
#define XDL_LINKER_SYM_DLOPEN_EXT_O  "__dl__ZL10dlopen_extPKciPK17android_dlextinfoPKv"
#define XDL_LINKER_SYM_DO_DLOPEN_O   "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv"
#define XDL_LINKER_SYM_LOADER_DLOPEN "__loader_dlopen"

typedef void *(*xdl_linker_dlopen_ext_t)(const char *, int, const void *, void *);
typedef void *(*xdl_linker_do_dlopen_t)(const char *, int, const void *, void *);
typedef void *(*xdl_linker_loader_dlopen_t)(const char *, int, const void *);

static pthread_mutex_t *xdl_linker_mutex = NULL;
static xdl_linker_dlopen_ext_t xdl_linker_dlopen_ext = NULL;
static xdl_linker_do_dlopen_t xdl_linker_do_dlopen = NULL;
static xdl_linker_loader_dlopen_t xdl_linker_loader_dlopen = NULL;

static void xdl_linker_init(void)
{
    static bool inited = false;
    if(inited) return;
    inited = true;

    void *handle = xdl_open(XDL_LINKER_BASENAME, XDL_DEFAULT);
    if(NULL == handle) return;

    int api_level = xdl_util_get_api_level();
    if(__ANDROID_API_L__ == api_level || __ANDROID_API_L_MR1__ == api_level)
    {
        // == Android 5.x
        xdl_linker_mutex = (pthread_mutex_t *)xdl_dsym(handle, XDL_LINKER_SYM_MUTEX, NULL);
    }
    else if(__ANDROID_API_N__ <= api_level && api_level <= __ANDROID_API_O_MR1__)
    {
        // == Android 7.x or 8.x
        bool is_n = (__ANDROID_API_N__ == api_level || __ANDROID_API_N_MR1__ == api_level);
        xdl_linker_dlopen_ext = (xdl_linker_dlopen_ext_t)xdl_dsym(handle, is_n ? XDL_LINKER_SYM_DLOPEN_EXT_N : XDL_LINKER_SYM_DLOPEN_EXT_O, NULL);
        if(NULL == xdl_linker_dlopen_ext)
        {
            xdl_linker_do_dlopen = (xdl_linker_do_dlopen_t)xdl_dsym(handle, is_n ? XDL_LINKER_SYM_DO_DLOPEN_N : XDL_LINKER_SYM_DO_DLOPEN_O, NULL);
            xdl_linker_mutex = (pthread_mutex_t *)xdl_dsym(handle, XDL_LINKER_SYM_MUTEX, NULL);
        }
    }
    else if(api_level >= __ANDROID_API_P__)
    {
        // >= Android 8.0
        xdl_linker_loader_dlopen = (xdl_linker_loader_dlopen_t)xdl_sym(handle, XDL_LINKER_SYM_LOADER_DLOPEN, NULL);
    }

    xdl_close(handle);
}

void xdl_linker_lock(void)
{
    xdl_linker_init();

    if(NULL != xdl_linker_mutex) pthread_mutex_lock(xdl_linker_mutex);
}

void xdl_linker_unlock(void)
{
    if(NULL != xdl_linker_mutex) pthread_mutex_unlock(xdl_linker_mutex);
}

void *xdl_linker_load(const char *filename)
{
    int api_level = xdl_util_get_api_level();

    if(api_level <= __ANDROID_API_M__)
    {
        // <= Android 6.0
        return dlopen(filename, RTLD_NOW);
    }
    else
    {
        xdl_linker_init();
        void *libc_func = (void *)snprintf;

        if(__ANDROID_API_N__ <= api_level && api_level <= __ANDROID_API_O_MR1__)
        {
            // == Android 7.x or 8.x
            if(NULL != xdl_linker_dlopen_ext)
                return xdl_linker_dlopen_ext(filename, RTLD_NOW, NULL, libc_func);
            else if(NULL != xdl_linker_do_dlopen)
            {
                xdl_linker_lock();
                void *handle = xdl_linker_do_dlopen(filename, RTLD_NOW, NULL, libc_func);
                xdl_linker_unlock();
                return handle;
            }
        }
        else
        {
            // >= Android 9.0
            if(NULL != xdl_linker_loader_dlopen)
                return xdl_linker_loader_dlopen(filename, RTLD_NOW, libc_func);
        }
    }

    return NULL;
}
