#include "bsp.h"
#include <easyflash.h>
#include <stdio.h>
#include <stdlib.h>

static void test_env(void);
static void test_env_1(void);
static void test_env_1_read(void);

int env_demo(void) {

    OS_ERR      err;
    
    if (easyflash_init() == EF_NO_ERR) 
        ;
    //test_env();
    //test_env_1();

    while(1) {
        /* test Env demo */
        test_env_1_read();
        
        OSTimeDly(5 * 1000, OS_OPT_TIME_DLY, &err); // 10s
    }

    return 0;
}

/**
 * Env demo.
 */
static void test_env(void) {
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
    assert_param(c_old_boot_times);
    i_boot_times = atol(c_old_boot_times);
    /* boot count +1 */
    i_boot_times ++;
    printf("The system now boot %d times\n\r", i_boot_times);
    /* interger to string */
    sprintf(c_new_boot_times,"%ld", i_boot_times);
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);
    ef_save_env();
    
}

static void test_env_1(void) {
    ef_set_and_save_env("SSID","administrator");
    ef_set_and_save_env("PASS","0123456789");
    ef_set_and_save_env("ACCOUNTID","abcdefghijmnopqrstuvwxyz0123456789");
    ef_set_and_save_env("PM_P0","0.123");
    ef_set_and_save_env("PM_P1","2.123");
    ef_set_and_save_env("0","{\"pm25\":25,\"co2\":600,\"voc\",10,\"temp\":23.6,\"hum\":60}");
    ef_set_and_save_env("1","{\"pm25\":26,\"co2\":700,\"voc\",20,\"temp\":25.6,\"hum\":40}");
}

static void test_env_1_read(void) {
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times;
    char *env_buf;

    c_old_boot_times = ef_get_env("boot_times");
    i_boot_times = atol(c_old_boot_times);
    printf("The system now boot %d times\n\r", i_boot_times);

    env_buf = ef_get_env("SSID");
    printf("[EasyFlash] SSID %s\n\r", env_buf);
    
    env_buf = ef_get_env("PASS");
    printf("[EasyFlash] PASS %s\n\r", env_buf);

    env_buf = ef_get_env("ACCOUNTID");
    printf("[EasyFlash] ACCOUNTID %s\n\r", env_buf);

    env_buf = ef_get_env("PM_P0");
    printf("[EasyFlash] PM_P0 %s\n\r", env_buf);

    env_buf = ef_get_env("PM_P1");
    printf("[EasyFlash] PM_P1 %s\n\r", env_buf);

    env_buf = ef_get_env("test_env");
    printf("[EasyFlash] test_env %s\n\r", env_buf);

    env_buf = ef_get_env("0");
    printf("[EasyFlash] Id_0 %s\n\r", env_buf);

    env_buf = ef_get_env("1");
    printf("[EasyFlash] Id_1 %s\n\r", env_buf);
}

                                                  