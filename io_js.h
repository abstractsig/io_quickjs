/*
 *
 * importing quickjs-2020-04-12 
 *
 * LICENSE
 * =======
 * See end of file for license terms.
 *
 */
#ifndef io_js_H_
#define io_js_H_
#include <io_core.h>

// fenv
#define FE_DOWNWARD		1
#define FE_TONEAREST		2
#define FE_TOWARDZERO	3
#define FE_UPWARD			4

int fesetround (int rdir);

#define CONFIG_BIGNUM
#define CONFIG_NO_ATOMICS

#define qjsrt_printf(rt,...)
#define qjsrt_putchar(rt,c)
#define qjsctx_printf(ctx,...) io_printf (JS_GetIO(ctx),##__VA_ARGS__);
#define qjsctx_putchar(ctx,c)

#define qjsp_printf(ps,...)

#define bf_printf(fmt,...)

#include "quickjs.h"

int io_quickjs_eval_buffer (JSContext*,const void*,int,const char*,int);

#ifdef IMPLEMENT_IO_JS
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------
#include <io_device.h>

/*
 *-----------------------------------------------------------------------------
 *
 * print values to log stream
 *
 *-----------------------------------------------------------------------------
 */
JSValue
io_quickjs_print (JSContext *ctx, JSValueConst this_val,int argc, JSValueConst *argv) {
	io_socket_t *log = io_get_socket (JS_GetIO(ctx),IO_LOG_SOCKET);
	io_encoding_t *print = io_socket_new_message (log);

	if (print) {
		const char *str;
		int i;

		for (i = 0; i < argc; i++) {
			if (i != 0) {
				io_encoding_append_byte (print,' ');
			}
				
			str = JS_ToCString(ctx, argv[i]);
			if (!str) {
				goto exception;
			}
			
			io_encoding_append_string (print,str,strlen(str));
			JS_FreeCString(ctx, str);
		}
	
		io_socket_send_message (log,print);
	}
	
	return JS_UNDEFINED;

exception:
	io_socket_send_message (log,print);
	return JS_EXCEPTION;
}

void
io_quickjs_dump_error (JSContext *ctx) {
	JSValue exception_val, val;
	bool is_error;

	exception_val = JS_GetException(ctx);
	is_error = JS_IsError(ctx, exception_val);

	if (!is_error) {
		qjsctx_printf (ctx,"Throw: ");
	}

	io_quickjs_print (ctx, JS_NULL, 1, (JSValueConst*) &exception_val);
	
	if (is_error) {
	  val = JS_GetPropertyStr(ctx, exception_val, "stack");
	  if (!JS_IsUndefined(val)) {
			const char *stack = JS_ToCString(ctx, val);
			qjsctx_printf (ctx,"%s\n", stack);
			JS_FreeCString(ctx, stack);
	  }
	  JS_FreeValue(ctx, val);
	}

	JS_FreeValue(ctx, exception_val);
}

int
io_quickjs_eval_buffer (
	JSContext *ctx,const void *buf,int buf_len,const char *name,int flags
) {
    JSValue val;
    int ret;

    val = JS_Eval(ctx, buf, buf_len, name, flags);
    if (JS_IsException(val)) {
        io_quickjs_dump_error (ctx);
        ret = -1;
    } else {
        ret = 0;
    }
    JS_FreeValue(ctx, val);
    return ret;
}

int
io_quickjs_enqueue_task (JSContext *ctx,JSJobFunc *task_func,int argc,JSValueConst *argv) {
	int r = JS_EnqueueJob(ctx,task_func,argc,argv);
	signal_io_task_pending (JS_GetIO(ctx));
	return r;
}


#include "quickjs.c"
#include "cutils.c"
#include "libbf.c"
#include "libregexp.c"
#include "libunicode.c"

#endif /* IMPLEMENT_IO_JS */
#endif
/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Gregor Bruce
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
