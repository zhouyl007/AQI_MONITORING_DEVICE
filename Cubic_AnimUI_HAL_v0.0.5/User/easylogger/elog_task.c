#include <elog.h>
#include "utils.h"

static void test_elog(void);
static void elog_user_assert_hook(const char* ex, const char* func, size_t line);

/**
 * Elog demo
 */
static void test_elog(void) {
    log_a("Hello EasyLogger!");
    log_e("Hello EasyLogger!");
    log_w("Hello EasyLogger!");
    log_i("Hello EasyLogger!");
    log_d("Hello EasyLogger!");
    log_v("Hello EasyLogger!");
    elog_raw("Hello EasyLogger!");
}

/**
 * System initialization thread.
 *
 * @param parameter parameter
 */
void log_init_thread(void){
	set_system_status(SYSTEM_STATUS_INIT);

    /* initialize EasyFlash and EasyLogger */
    if ((elog_init() == ELOG_NO_ERR)) {
        /* set enabled format */
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
        elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
        /* set EasyLogger assert hook */
        elog_assert_set_hook(elog_user_assert_hook);
        /* start EasyLogger */
        elog_start();
        /* initialize OK and switch to running status */
        set_system_status(SYSTEM_STATUS_RUN);
    } else {
        /* initialize fail and switch to fault status */
        set_system_status(SYSTEM_STATUS_FAULT);
    }
}

static void elog_user_assert_hook(const char* ex, const char* func, size_t line) {

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    /* disable async output */
    elog_async_enabled(false);
#endif

    /* disable logger output lock */
    elog_output_lock_enabled(false);
    /* output logger assert information */
    elog_a("elog", "(%s) has assert failed at %s:%ld.\n", ex, func, line);
    while(1);
}


