/*
 *
 * std module
 *
 * LICENSE
 * =======
 * See end of file for license terms.
 *
 */
#ifndef io_js_std_module_H_
#define io_js_std_module_H_
#include <io_js.h>

void io_js_standard_module (JSContext*);

#ifdef IMPLEMENT_JS_IO
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------

typedef struct {
	bool has_object;
	JSContext *ctx;
	JSValue resolve;
	JSValue reject;
	
	io_alarm_t alarm;
	io_event_t on_timeout;
	io_event_t on_error;
	
} JS_IOTimer;

static JSClassID io_js_timer_class_id = 0;

static void
free_io_js_timer (JSRuntime *rt, JS_IOTimer *timer) {
	JS_FreeValueRT (rt,timer->resolve);
	JS_FreeValueRT (rt,timer->reject);
	js_free_rt (rt,timer);
}

static void
io_js_timer_finalizer (JSRuntime *rt, JSValue val) {
	JS_IOTimer *tmr = JS_GetOpaque(val, io_js_timer_class_id);
	if (tmr) {
		tmr->has_object = false;
		if (!is_io_alarm_active (&tmr->alarm))
			free_io_js_timer(rt, tmr);
	} else {
		// something bad happened
	}
}

static void
io_js_timer_mark (JSRuntime *rt, JSValueConst val,JS_MarkFunc *mark_func) {
	JS_IOTimer *timer = JS_GetOpaque(val, io_js_timer_class_id);
	if (timer) {
		JS_MarkValue (rt, timer->resolve, mark_func);
	} else {
		// something bad happened
	}
}

static JSClassDef io_js_timer_class = {
    "IOTimer",
    .finalizer = io_js_timer_finalizer,
    .gc_mark = io_js_timer_mark,
}; 

static void
io_js_call_handler (JSContext *ctx,JSValueConst func) {
	JSValue ret, func1;
	func1 = JS_DupValue(ctx, func);
	ret = JS_Call(ctx, func1, JS_UNDEFINED, 0, NULL);
	JS_FreeValue(ctx, func1);

	if (JS_IsException(ret)) {
		io_js_dump_error (ctx);
	}
	
	JS_FreeValue (ctx, ret);
}

static JSValue
io_js_call_timer_continuation (JSContext *ctx, int argc, JSValueConst *argv) {
	io_js_call_handler(ctx,argv[0]);
	return JS_UNDEFINED;
}

static void
io_js_setTimeout_on_timeout (io_event_t *ev) {
	JS_IOTimer *tmr = ev->user_value;
	JSContext *ctx = tmr->ctx;
	JSValue func = tmr->resolve;
	
	tmr->resolve = JS_UNDEFINED;
	
	//
	// if object has been freed we need to free timer
	//
	if (!tmr->has_object) {
		free_io_js_timer (JS_GetRuntime(ctx),tmr);
	}

	io_js_enqueue_task (ctx,io_js_call_timer_continuation,1,&func);
	JS_FreeValue(ctx, func);
}

static void
io_js_setTimeout_on_error (io_event_t *ev) {
	JS_IOTimer *tmr = ev->user_value;
	JSContext *ctx = tmr->ctx;

	// needs implementation
	io_panic (JS_GetIO(ctx),IO_PANIC_UNRECOVERABLE_ERROR);	
}

static JSValue
io_js_setTimeout (
	JSContext *ctx, JSValueConst this_val,int argc, JSValueConst *argv
) {
	int64_t delay;
	JSValueConst func;
	JS_IOTimer *tmr;
	JSValue obj;

	func = argv[0];
	if (!JS_IsFunction(ctx, func))
	  return JS_ThrowTypeError(ctx, "not a function");
	  
	if (JS_ToInt64(ctx, &delay, argv[1]))
	  return JS_EXCEPTION;
	  
	obj = JS_NewObjectClass(ctx, io_js_timer_class_id);
	if (JS_IsException(obj))
	  return obj;
	  
	tmr = js_mallocz(ctx, sizeof(JS_IOTimer));
	if (!tmr) {
	  JS_FreeValue(ctx, obj);
	  return JS_EXCEPTION;
	}
	
	tmr->ctx = ctx;
	tmr->resolve = JS_DupValue(ctx, func);
	tmr->reject = JS_UNDEFINED;

	initialise_io_event (&tmr->on_timeout,io_js_setTimeout_on_timeout,tmr);
	initialise_io_event (&tmr->on_error,io_js_setTimeout_on_error,tmr);
	initialise_io_alarm (
		&tmr->alarm,&tmr->on_timeout,&tmr->on_error,time_zero()
	);

	set_alarm_delay_time (JS_GetIO(ctx),&tmr->alarm,millisecond_time(delay));

	io_enqueue_alarm (JS_GetIO(ctx),&tmr->alarm);

	tmr->has_object = true;
	JS_SetOpaque (obj,tmr);
	
	return obj;
}

static void
io_js_after_on_timeout (io_event_t *ev) {
	JS_IOTimer *timer = ev->user_value;
	JSContext *ctx = timer->ctx;
	io_js_enqueue_task (ctx,io_js_call_timer_continuation,1,&timer->resolve);
	free_io_js_timer (JS_GetRuntime (ctx),timer);
}

static void
io_js_after_on_error (io_event_t *ev) {
	JS_IOTimer *timer = ev->user_value;
	JSContext *ctx = timer->ctx;

	// needs implementation
	io_panic (JS_GetIO(ctx),IO_PANIC_UNRECOVERABLE_ERROR);	
}

static JSValue
io_js_after (
	JSContext *ctx, JSValueConst this_val,int argc, JSValueConst *argv
) {
	JSValue resolving_funcs[2];
	JSValue after = JS_NewPromiseCapability(ctx,resolving_funcs);
	int64_t delay;
	JS_IOTimer *tmr;

	if (JS_ToInt64(ctx, &delay, argv[0]))
	  return JS_EXCEPTION;

	tmr = js_mallocz(ctx, sizeof(JS_IOTimer));
	if (!tmr) {
	  JS_FreeValue(ctx, after);
	  return JS_EXCEPTION;
	}

	tmr->ctx = ctx;
	tmr->resolve = resolving_funcs[0];
	tmr->reject = resolving_funcs[1];

	initialise_io_event (&tmr->on_timeout,io_js_after_on_timeout,tmr);
	initialise_io_event (&tmr->on_error,io_js_after_on_error,tmr);
	initialise_io_alarm (
		&tmr->alarm,&tmr->on_timeout,&tmr->on_error,time_zero()
	);
	set_alarm_delay_time (JS_GetIO(ctx),&tmr->alarm,millisecond_time(delay));

	io_enqueue_alarm (JS_GetIO(ctx),&tmr->alarm);

	return after;
}

static JSValue
io_js_clearTimeout (
	JSContext *ctx, JSValueConst this_val,int argc, JSValueConst *argv
) {
	JS_IOTimer *th = JS_GetOpaque2(ctx, argv[0], io_js_timer_class_id);
	if (th) {
		io_dequeue_alarm (JS_GetIO(ctx),&th->alarm);
		return JS_UNDEFINED;
	} else {
		return JS_EXCEPTION;
	}
}

static JSValue
io_js_eval_string (
	JSContext *ctx,JSValueConst this_val,int argc,JSValueConst *argv
) {
	const char *str;
	size_t len;
	str = JS_ToCStringLen(ctx, &len, argv[0]);
	if (str) {
		JSValue ret = JS_Eval(ctx, str, len, "<eval>", JS_EVAL_TYPE_GLOBAL);
		JS_FreeCString(ctx, str);
		return ret;
	} else {
		return JS_EXCEPTION;
	}
}

const JSCFunctionListEntry io_js_std_funcs[] = {
	JS_CFUNC_DEF("setTimeout",		2,io_js_setTimeout),
	JS_CFUNC_DEF("clearTimeout",	1,io_js_clearTimeout),
	JS_CFUNC_DEF("evaluate",		1,io_js_eval_string),
	JS_CFUNC_DEF("after",			1,io_js_after),
};

static int
io_js_std_initialise (JSContext *ctx, JSModuleDef *m) {
/*
	JSValue obj = JS_NewCFunction2 (
		ctx, js_std_error_constructor,"Error", 1, JS_CFUNC_constructor, 0
	);
	JS_SetPropertyFunctionList(ctx, obj, js_std_error_funcs,countof(js_std_error_funcs));
	JS_SetModuleExport(ctx, m, "Error", obj);
*/

	JS_NewClassID (&io_js_timer_class_id);
	JS_NewClass (JS_GetRuntime(ctx), io_js_timer_class_id, &io_js_timer_class);

	JS_SetModuleExport(ctx, m, "global", JS_GetGlobalObject(ctx));

	return JS_SetModuleExportList (
		ctx, m, io_js_std_funcs,SIZEOF(io_js_std_funcs)
	);
}

static JSModuleDef*
io_js_init_module_std (JSContext *ctx,const char *module_name) {
	JSModuleDef *m = JS_NewCModule(ctx, module_name, io_js_std_initialise);
	if (m) {
		JS_AddModuleExportList(ctx, m, io_js_std_funcs, SIZEOF(io_js_std_funcs));
		JS_AddModuleExport(ctx, m, "global");
		//JS_AddModuleExport(ctx, m, "Error");
	}
	
	return m;
}

void
io_js_standard_module (JSContext *ctx) {
	const char *str = (
		"import * as std from 'std';\n"
		"std.global.std = std;\n"
	);
	io_js_init_module_std (ctx,"std");
	io_js_eval_buffer (ctx, str, strlen(str),"<std>",JS_EVAL_TYPE_MODULE);
}


#endif /* IMPLEMENT_JS_IO */
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
