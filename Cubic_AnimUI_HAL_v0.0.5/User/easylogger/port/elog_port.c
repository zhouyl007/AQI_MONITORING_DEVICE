/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
 
#include <elog.h>
#include <elog_flash.h>
#include  <app_cfg.h>
#include "bsp.h"

extern void  SENSORTaskPend (void);
extern void  SENSORTaskPost (void);

#ifdef ELOG_ASYNC_OUTPUT_ENABLE

static  OS_SEM   output_notice;
static  OS_TCB   AppTaskASYNCTCB;
static  CPU_STK  AppTaskASYNCStk[APP_CFG_TASK_ASYNC_STK_SIZE];

static void async_output(void *arg);

#endif


/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {

    OS_ERR      err;
    ElogErrCode result = ELOG_NO_ERR;

#ifdef ELOG_ASYNC_OUTPUT_ENABLE

    /**************创建 ASYNC 任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskASYNCTCB,             
                 (CPU_CHAR     *)"App Task UserIF",
                 (OS_TASK_PTR   )async_output, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_ASYNC_PRIO,
                 (CPU_STK      *)&AppTaskASYNCStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_ASYNC_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_ASYNC_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

    /******************* ASYNC 信号量 ********************/
	OSSemCreate((OS_SEM    *)&output_notice,
				(CPU_CHAR  *)"async_output_notice",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
    
#endif
 
    return result;
}


static void  async_sem_take ( void)
{
	OS_ERR      err;

	OSSemPend((OS_SEM  *)&output_notice,
              (OS_TICK  )1000u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&err);            
}

static void  async_sem_release (void)
{
	OS_ERR         err;

	(void)OSSemPost((OS_SEM  *)&output_notice,
                  	 (OS_OPT   )OS_OPT_POST_1,
                  	 (OS_ERR  *)&err); 	
}
/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    
    /* output to terminal */
    printf("%.*s", size, log);
    /* output to flash */
    //elog_flash_write(log, size); 
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    
   SENSORTaskPend();
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    
   SENSORTaskPost(); 
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    
   static char cur_system_time[16] = { 0 };
   snprintf(cur_system_time, 16, "tick:%010d", HAL_GetTick());
   return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {

        return ""; 
}

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
void elog_async_output_notice(void) {
    async_sem_release();
}

static void async_output(void *arg) {
    size_t get_log_size = 0;
    static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];

    while(TRUE) {
        /* waiting log */
        async_sem_take();
        /* polling gets and outputs the log */
        while(TRUE) {

#ifdef ELOG_ASYNC_LINE_OUTPUT
            get_log_size = elog_async_get_line_log(poll_get_buf, sizeof(poll_get_buf));
#else
            get_log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
#endif

            if (get_log_size) {
                elog_port_output(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
}
#endif


