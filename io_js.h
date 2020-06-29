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
#define qjsctx_printf(ctx,...)
#define qjsctx_putchar(ctx,c)

#define qjsp_printf(ps,...)

#define bf_printf(fmt,...)

#include "quickjs.h"

#ifdef IMPLEMENT_IO_CORE
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------

#include "quickjs.c"
#include "cutils.c"
#include "libbf.c"
#include "libregexp.c"
#include "libunicode.c"

#endif /* IMPLEMENT_IO_CORE */
#ifdef IMPLEMENT_VERIFY_QUICKJS
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------
TEST_BEGIN(test_quickjs_create_1) {
	memory_info_t bminfo_begin,bminfo_end;

	JSRuntime *jsfrt;
	JSContext *jsfctx;

	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_begin);

	jsfrt = JS_NewRuntime(TEST_IO);
	jsfctx = JS_NewContextRaw(jsfrt);

	JS_SetGCThreshold (jsfrt,-1);

	io_byte_memory_get_info (io_get_byte_memory(TEST_IO),&bminfo_end);
	VERIFY(bminfo_end.used_bytes > bminfo_begin.used_bytes,NULL);

	JS_FreeContext(jsfctx);
	JS_FreeRuntime(jsfrt);

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
