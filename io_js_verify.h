/*
 *
 * importing quickjs-2020-04-12 
 *
 * LICENSE
 * =======
 * See end of file for license terms.
 *
 */
#ifndef io_js_verify_H_
#define io_js_verify_H_
#include <io_verify.h>
#include "io_js.h"
#ifdef IMPLEMENT_VERIFY_QUICKJS
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------
TEST_BEGIN(test_quickjs_create_1) {
	memory_info_t bminfo_begin,bminfo_end;

	JSRuntime *rt;
	JSContext *ctx;

	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_begin);

	rt = JS_NewRuntime(TEST_IO);
	ctx = JS_NewContextRaw(rt);

	JS_SetGCThreshold (rt,-1);
	
	JS_AddIntrinsicBaseObjects (ctx);
	JS_AddIntrinsicJSON (ctx);
	JS_AddIntrinsicRegExpCompiler (ctx);
	JS_AddIntrinsicRegExp (ctx);
	JS_AddIntrinsicPromise (ctx);
	JS_AddIntrinsicTypedArrays (ctx);
	JS_AddIntrinsicProxy (ctx);
	JS_AddIntrinsicEval (ctx);

	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_end);
	VERIFY(bminfo_end.used_bytes > bminfo_begin.used_bytes,NULL);

	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);

	io_log_flush(TEST_IO);
	
	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_end);
	VERIFY(bminfo_end.used_bytes == bminfo_begin.used_bytes,NULL);
}
TEST_END

uint32_t test_quickjs_eval_1_result;

static JSValue
test_result (
	JSContext *ctx, JSValueConst this_value,int argc, JSValueConst *argv
) {
	test_quickjs_eval_1_result = 1;	
	return JS_UNDEFINED;
}

static const JSCFunctionListEntry global_functions[] = {
	JS_CFUNC_DEF("VERIFY",1,test_result),
};

void
io_js_add_global_test_functions (JSContext *ctx) {
	JSValue global_ns = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr (
		ctx,global_ns,"VERIFY",JS_NewCFunction(ctx, test_result, "VERIFY", 1)
	);
	JS_FreeValue(ctx, global_ns);
}

TEST_BEGIN(test_quickjs_eval_1) {
	memory_info_t bminfo_begin,bminfo_end;

	JSRuntime *rt;
	JSContext *ctx;
	const char *begin = ""
		//"print(io.byte_memory_used + ' bytes of '  + io.byte_memory_total + \"\\n\");"
		"VERIFY(true);"
	;
 
	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_begin);

	rt = JS_NewRuntime(TEST_IO);
	ctx = JS_NewContextRaw(rt);

	JS_SetGCThreshold (rt,-1);
	
	JS_AddIntrinsicBaseObjects (ctx);
	JS_AddIntrinsicEval (ctx);
	io_js_add_helpers (ctx);
	io_js_standard_module (ctx);
	io_js_add_global_test_functions (ctx);
	io_js_io_namespace (ctx,NULL);

	test_quickjs_eval_1_result = 0;
	io_js_eval_buffer (ctx,begin,strlen(begin),"<test>",0);
	VERIFY (test_quickjs_eval_1_result == 1,NULL);
	
	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_end);
	VERIFY(bminfo_end.used_bytes > bminfo_begin.used_bytes,NULL);

	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);

	io_log_flush(TEST_IO);
	
	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_end);
	VERIFY(bminfo_end.used_bytes == bminfo_begin.used_bytes,NULL);
}
TEST_END

UNIT_SETUP(setup_quickjs_unit_test) {
	io_value_memory_get_info (io_get_short_term_value_memory (TEST_IO),TEST_MEMORY_INFO);
	io_byte_memory_get_info (io_get_byte_memory (TEST_IO),TEST_MEMORY_INFO + 1);
	return VERIFY_UNIT_CONTINUE;
}

UNIT_TEARDOWN(teardown_quickjs_unit_test) {
	io_value_memory_t *vm = io_get_short_term_value_memory (TEST_IO);
	memory_info_t bm_end,vm_end;
	io_value_memory_do_gc (vm,-1);
	io_value_memory_get_info (vm,&vm_end);
	VERIFY (vm_end.used_bytes == TEST_MEMORY_INFO[0].used_bytes,NULL);
	io_byte_memory_get_info (io_get_byte_memory (TEST_IO),&bm_end);
	VERIFY (bm_end.used_bytes == TEST_MEMORY_INFO[1].used_bytes,NULL);
}

static void
quickjs_unit_test (V_unit_test_t *unit) {
	static V_test_t const tests[] = {
		test_quickjs_create_1,
		test_quickjs_eval_1,
		0
	};
	unit->name = "quickjs";
	unit->description = "quickjs unit test";
	unit->tests = tests;
	unit->setup = setup_quickjs_unit_test;
	unit->teardown = teardown_quickjs_unit_test;
}

void
run_ut_quickjs (V_runner_t *runner) {
	static const unit_test_t test_set[] = {
		quickjs_unit_test,
		0
	};
	V_run_unit_tests(runner,test_set);
}

#endif /* IMPLEMENT_VERIFY_QUICKJS */
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
