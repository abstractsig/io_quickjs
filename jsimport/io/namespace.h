/*
 *
 * io module
 *
 * LICENSE
 * =======
 * See end of file for license terms.
 *
 */
#ifndef io_js_io_namespace_H_
#define io_js_io_namespace_H_
#include <io_js.h>
#include "js_io_socket.h"

typedef struct  {
	const char *name;
	int handle;
	void	(*constructor) (JSContext*,JSValue,const char*,int);
	const char *setup;
} js_io_socket_def_t;

#define END_OF_JS_IO_SOCKETS			{NULL}
#define IS_LAST_JS_IO_SOCKET(s)		((s)->name == NULL)

typedef struct io_js_filesystem_def {
	const char *name;
	void const *config; // for lfs
	const char *setup;
} js_io_filesystem_def_t;

typedef struct io_js_pin_def {
	const char *name;
	io_pin_t pin;
	const char *setup;
	void (*configure) (io_t*,io_pin_t);
} js_io_pin_def_t;

#define END_OF_PINS	{NULL}
#define IS_LAST_PIN(p) ((p)->name == NULL)

typedef struct io_js_config {
	const js_io_pin_def_t* pins;
	const js_io_socket_def_t* sockets;
	const js_io_filesystem_def_t* filesystems;
} io_js_device_configuration_t;

void io_js_io_namespace (JSContext*,io_js_device_configuration_t const*);
//void io_js_io_module (JSContext*,io_js_device_configuration_t const*,bool,char const**,const char*);


#ifdef IMPLEMENT_IO_JS
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------

static JSValue
io_js_io_add_sockets (
	JSContext *ctx,JSValue sockets_ns,const js_io_socket_def_t sockets[]
) {
	js_io_socket_def_t const *def = sockets;
	
	while (!IS_LAST_JS_IO_SOCKET(def)) {
		def->constructor(ctx,sockets_ns,def->name,def->handle);
		def++;
	}
	
	return sockets_ns;
}

static JSValue 
io_js_io_byte_memory_used (JSContext *ctx,JSValueConst this_value) {
	memory_info_t bminfo;
	char buffer[64];
	io_byte_memory_get_info (io_get_byte_memory(JS_GetIO (ctx)),&bminfo);
	uint32_t len = stbsp_snprintf (
		buffer,sizeof(buffer),"%u",bminfo.used_bytes
	);
	return JS_NewStringLen(ctx,buffer,len);
}

static JSValue 
io_js_io_byte_memory_total (JSContext *ctx,JSValueConst this_value) {
	memory_info_t bminfo;
	char buffer[64];
	io_byte_memory_get_info (io_get_byte_memory(JS_GetIO (ctx)),&bminfo);
	uint32_t len = stbsp_snprintf (
		buffer,sizeof(buffer),"%u",bminfo.total_bytes
	);
	return JS_NewStringLen(ctx,buffer,len);
}

const JSCFunctionListEntry io_js_io_functions[] = {
	JS_CGETSET_DEF("byte_memory_used"	,io_js_io_byte_memory_used,NULL),
	JS_CGETSET_DEF("byte_memory_total"	,io_js_io_byte_memory_total,NULL),
};

void
io_js_io_namespace (JSContext *ctx,io_js_device_configuration_t const* config) {
	JSValue global_ns = JS_GetGlobalObject(ctx);
	JSValue io_ns = JS_NewObject(ctx);

	JS_SetPropertyFunctionList (
		ctx,io_ns,io_js_io_functions,SIZEOF(io_js_io_functions)
	);

	JS_SetPropertyStr(ctx,global_ns,"io",io_ns);

	if (config) {
		JSValue sockets = JS_UNDEFINED;
		JSValue pins = JS_UNDEFINED;

		if (config->sockets) {
			sockets =io_js_io_add_sockets (ctx,JS_NewObject(ctx),config->sockets);
		} 
		JS_SetPropertyStr (ctx,io_ns,"socket",sockets);
		JS_SetPropertyStr (ctx,io_ns,"pin",pins);
	}
	
	JS_FreeValue(ctx,global_ns);
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
