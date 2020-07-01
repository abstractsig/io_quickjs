/*
 *
 * quickJS wrapper for io_sockets
 *
 * LICENSE
 * =======
 * See end of file for license terms.
 *
 */
#ifndef js_io_socket_H_
#define js_io_socket_H_
#include <io_js.h>

typedef struct {
	JSContext *ctx;
	JSValue self;
	
	int handle;
	
	JSValue receive_callback;
	io_event_t received_data_available;
	io_event_t transmit_available;

	JSValue async_gets[2];
	
	io_address_t address;
		
} io_js_io_socket_t;

#ifdef IMPLEMENT_IO_JS
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------

static JSClassID js_io_socket_class_id = 0;

static void
js_io_socket_finalizer (JSRuntime *rt, JSValue val) {
	io_js_io_socket_t *js_io_socket = JS_GetOpaque(val, js_io_socket_class_id);
	if (js_io_socket) {
		JS_FreeValueRT(rt,js_io_socket->async_gets[0]);
		JS_FreeValueRT(rt,js_io_socket->async_gets[1]);
		JS_FreeValueRT(rt,js_io_socket->receive_callback);
		js_free_rt (rt,js_io_socket);
	} else {
		io_panic (JS_GetIOFromRT (rt),IO_PANIC_SOMETHING_BAD_HAPPENED);
	}
}

static void
js_io_socket_mark (JSRuntime *rt, JSValueConst val,JS_MarkFunc *mark_func) {
	io_js_io_socket_t *js_io_socket = JS_GetOpaque(val, js_io_socket_class_id);
	if (js_io_socket) {
		JS_MarkValue (rt,js_io_socket->receive_callback, mark_func);
		JS_MarkValue (rt,js_io_socket->async_gets[0], mark_func);
		JS_MarkValue (rt,js_io_socket->async_gets[1], mark_func);
	} else {
		io_panic (JS_GetIOFromRT (rt),IO_PANIC_SOMETHING_BAD_HAPPENED);
	}
}

static JSClassDef js_io_socket_class = {
    "js_io_socket",
    .finalizer = js_io_socket_finalizer,
    .gc_mark = js_io_socket_mark,
}; 

static JSValue
js_io_socket_close (
	JSContext *ctx, JSValueConst this_value,int argc, JSValueConst *argv
) {
	io_js_io_socket_t *js_io_socket = JS_GetOpaque2(ctx,this_value,js_io_socket_class_id);
	if (js_io_socket) {
		io_socket_t *socket = io_get_socket (JS_GetIO(ctx),js_io_socket->handle);
		if (socket) {
			io_socket_close (socket);
		}
	} else {
		io_panic (JS_GetIO (ctx),IO_PANIC_SOMETHING_BAD_HAPPENED);
	}
	
	return JS_UNDEFINED;
}

static JSValue
js_io_socket_open (
	JSContext *ctx, JSValueConst this_value,int argc, JSValueConst *argv
) {
	io_js_io_socket_t *js_io_socket = JS_GetOpaque2(ctx,this_value,js_io_socket_class_id);
	if (js_io_socket) {
		io_socket_t *socket = io_get_socket (JS_GetIO(ctx),js_io_socket->handle);
		if (socket) {
			if (io_socket_open (socket,IO_SOCKET_OPEN_CONNECT)) {
				io_socket_bind_inner (
					socket,
					js_io_socket->address,
					&js_io_socket->transmit_available,
					&js_io_socket->received_data_available
				);
				return JS_TRUE;
			}
		} else {
			io_panic (JS_GetIO (ctx),IO_PANIC_SOMETHING_BAD_HAPPENED);
		}
	} else {
		io_panic (JS_GetIO (ctx),IO_PANIC_SOMETHING_BAD_HAPPENED);
	}
	
	return JS_FALSE;
}

static JSValue
js_io_socket_print (
	JSContext *ctx, JSValueConst this_value,int argc, JSValueConst *argv
) {
	io_js_io_socket_t *js_io_socket = JS_GetOpaque2(ctx,this_value,js_io_socket_class_id);
	if (js_io_socket) {
		io_socket_t *socket = io_get_socket (JS_GetIO(ctx),js_io_socket->handle);
		
		if (socket) {
			io_encoding_t *encoding = io_socket_new_message (socket);
			if (encoding) {
				int i;

				for(i = 0; i < argc; i++) {
					const char *str;
					if (i != 0) {
						io_encoding_append_byte (encoding,' ');
					}
					if ((str = JS_ToCString(ctx, argv[i])) != NULL) {
						io_encoding_append_string (encoding,str,-1);
						JS_FreeCString(ctx, str);
					} else {
						unreference_io_encoding (encoding);
						return JS_EXCEPTION;
					}
				}
				io_encoding_append_byte (encoding,'\n');
				if (io_socket_send_message (socket,encoding)) {
					return JS_TRUE;
				} else {
					return JS_FALSE;
				}
			}
		}
	}
	
	return JS_UNDEFINED;
}

static JSValue
js_io_socket_send (
	JSContext *ctx, JSValueConst this_value,int argc, JSValueConst *argv
) {
	
	return JS_UNDEFINED;
}

static JSValue 
js_io_socket_get_receive (JSContext *ctx, JSValueConst this_value) {
	io_js_io_socket_t *js_io_socket = JS_GetOpaque2(ctx,this_value,js_io_socket_class_id);
	if (js_io_socket) {
		return JS_DupValue(ctx,js_io_socket->receive_callback);
	} else {
		io_panic (JS_GetIO (ctx),IO_PANIC_SOMETHING_BAD_HAPPENED);
	}
	return JS_UNDEFINED;
}

static JSValue
js_io_socket_set_receive (
	JSContext *ctx,JSValueConst this_value,JSValueConst new_value
) {
	io_js_io_socket_t *js_io_socket = JS_GetOpaque2 (
		ctx,this_value,js_io_socket_class_id
	);
	if (js_io_socket) {
		if (JS_IsFunction(ctx,new_value)) {
			JS_FreeValue (ctx,js_io_socket->receive_callback);
			js_io_socket->receive_callback = JS_DupValue(ctx,new_value);
		} else {
			return JS_ThrowTypeError (ctx,"not a function");
		}
	} else {
		io_panic (JS_GetIO (ctx),IO_PANIC_SOMETHING_BAD_HAPPENED);
	}
	
	return JS_UNDEFINED;
}

/*
 *-----------------------------------------------------------------------------
 *
 * jsf_io_log_gets --
 *
 * e.g. await this.gets(200);
 *
 *-----------------------------------------------------------------------------
 */
static JSValue
js_io_socket_gets (
	JSContext *ctx,JSValueConst this_value,int argc,JSValueConst *argv
) {
	io_js_io_socket_t *this = JS_GetOpaque2 (
		ctx,this_value,js_io_socket_class_id
	);
	if (this) {
		if (JS_IsUndefined(this->async_gets[0])) {
			return JS_NewPromiseCapability (ctx,this->async_gets);
		} else {
			return JS_ThrowTypeError (ctx,"only one gets allowed");
		}		
	} else {
		io_panic (JS_GetIO (ctx),IO_PANIC_SOMETHING_BAD_HAPPENED);
	}
	
	return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_io_socket_functions[] = {
	JS_CFUNC_DEF("close",			0,js_io_socket_close),
	JS_CFUNC_DEF("gets",				1,js_io_socket_gets),
	JS_CFUNC_DEF("print",			1,js_io_socket_print),
	JS_CFUNC_DEF("send",				1,js_io_socket_send),
	JS_CFUNC_DEF("open",				0,js_io_socket_open),
	JS_CGETSET_DEF("on_receive"	 ,js_io_socket_get_receive,js_io_socket_set_receive),
};

void
js_io_add_io_socket_class (JSContext *ctx) {
	JSValue prototype;
	
	JS_NewClassID (&js_io_socket_class_id);
	JS_NewClass (
		JS_GetRuntime(ctx), js_io_socket_class_id, &js_io_socket_class
	);

	prototype = JS_NewObject(ctx);

	JS_SetPropertyFunctionList (
		ctx,prototype,js_io_socket_functions,SIZEOF(js_io_socket_functions)
	);
	
	JS_SetClassProto (ctx,js_io_socket_class_id,prototype);
}

static JSValue
js_io_socket_continuation (JSContext *ctx, int argc, JSValueConst *argv) {
	JSValue result, function;
	function = JS_DupValue (ctx, argv[0]);
	result = JS_Call (ctx,function,JS_UNDEFINED,1,argv + 1);
	JS_FreeValue (ctx,function);

	if (JS_IsException (result)) {
		io_js_dump_error (ctx);
	}
	
	JS_FreeValue (ctx,result);
	return JS_UNDEFINED;
}

static void
js_io_socket_read_bytes (io_event_t *ev) {
	io_js_io_socket_t *this = ev->user_value;
	JSContext *ctx = this->ctx;
	io_socket_t *socket = io_get_socket (JS_GetIO(ctx),this->handle);
	
	if (socket) {
		io_encoding_pipe_t* rx = cast_to_io_encoding_pipe (
			io_socket_get_receive_pipe (
				socket,this->address
			)
		);
		if (rx) {
			io_encoding_t *next;
			if (io_encoding_pipe_peek (rx,&next)) {
				
				const uint8_t *b,*e;	
				JSValue argv[2] = {
					this->async_gets[0],
				};

				io_encoding_get_content (next,&b,&e);
				argv[1] = JS_NewStringLen(ctx,(const char*) b,e - b);

				io_js_enqueue_task (ctx,js_io_socket_continuation,SIZEOF(argv),argv);

				JS_FreeValue(ctx,argv[1]);
				JS_FreeValue(ctx,this->async_gets[0]);
				JS_FreeValue(ctx,this->async_gets[1]);
				this->async_gets[0] = JS_UNDEFINED;
				this->async_gets[1] = JS_UNDEFINED;
			
				io_encoding_pipe_pop_encoding (rx);
			}
		}
	}
}

void
js_io_socket_constructor (JSContext *ctx,JSValue ns,const char *name,int id) {
	JSValue obj;
	io_js_io_socket_t *js_io_socket;
	io_socket_t *socket;

	socket = io_get_socket (JS_GetIO (ctx),id);
	if (!socket) {
		return;
	}
	
	if (js_io_socket_class_id == 0) {
		js_io_add_io_socket_class (ctx);
	}
	
	obj = JS_NewObjectClass(ctx,js_io_socket_class_id);
	if (JS_IsException(obj))
		return;

	js_io_socket = js_mallocz(ctx, sizeof(io_js_io_socket_t));
	js_io_socket->ctx = ctx;
	js_io_socket->handle = id;
	js_io_socket->receive_callback = JS_UNDEFINED;
	js_io_socket->self = obj;
	
	js_io_socket->async_gets[0] = JS_UNDEFINED;
	js_io_socket->async_gets[1] = JS_UNDEFINED;
	
	initialise_io_event (
		&js_io_socket->received_data_available,js_io_socket_read_bytes,js_io_socket
	);
	
//	add_log_stream_properties (ctx,js_io_socket,obj);
	
	JS_SetOpaque (obj,js_io_socket);
	JS_SetPropertyStr(ctx,ns,name,obj);
}

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
