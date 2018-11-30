#include "bsp.h"

unsigned char key_driver(void) 
{     
    static unsigned char key_state = KEY_STATE_0;         	// 按键状态变量
    static unsigned int key_time   = 0;           	// 按键计时变量
    unsigned char key_press;
	unsigned char key_return       = N_KEY;  				// 清除 返回按键值
	static uint8_t key_ctr = 0;
	
    key_press = GetTouchKeyState();             	// 读取当前键值

    switch (key_state){     
        case KEY_STATE_0:                       	// 按键状态0：判断有无按键按下
            if (!key_press){                    	// 有按键按下
                key_time = 0;                   	// 清零时间间隔计数
                key_state = KEY_STATE_1;        	// 然后进入 按键状态1
            }        
            break;
        case KEY_STATE_1:                       	// 按键状态1：软件消抖（确定按键是否有效，而不是误触）。按键有效的定义：按键持续按下超过设定的消抖时间。
            if (!key_press){
                
                key_time++;                     	// 一次10ms
                
                if(key_time >= SINGLE_KEY_TIME){ 	// 消抖时间为：SINGLE_KEY_TIME*10ms = 30ms;
                    key_state = KEY_STATE_2;    	// 如果按键时间超过 消抖时间，即判定为按下的按键有效。按键有效包括两种：单击或者长按，进入 按键状态2， 继续判定到底是那种有效按键
                }
			}
            else 
				key_state = KEY_STATE_0;       		// 如果按键时间没有超过，判定为误触，按键无效，返回 按键状态0，继续等待按键
            break; 
        case KEY_STATE_2:                       	// 按键状态2：判定按键有效的种类：是单击，还是长按
            if(key_press == TRUE && key_ctr == FALSE){                      	// 如果按键在设定的长按时间内释放，则判定为单击
                 key_return = S_KEY;            	// 返回有效按键值：单击
                 key_state = KEY_STATE_0;       	// 返回按键状态0，继续等待按键
            }
            else{
                key_time++;
                
                if(key_time >= MIDDLE_KEY_TIME){
                   key_state = KEY_STATE_4;
                   key_ctr = TRUE;
                }
                
                if(key_time >= LONG_KEY_TIME){   	// 如果按键时间超过 设定的长按时间（LONG_KEY_TIME*10ms=200*10ms=2000ms）, 则判定为 长按
                    key_return = L_KEY;          	// 返回 有效键值值：长按
                    key_state = KEY_STATE_3;     	// 去状态3，等待按键释放
                }  
            }
            break;
      case KEY_STATE_3:                         
          if (key_press){ 							// 等待按键释放
              key_time = NULL;
              key_ctr = FALSE;
              key_state = KEY_STATE_0;          	// 按键释放后，进入 按键状态0 ，进行下一次按键的判定
          }
           break;
      case KEY_STATE_4:
         if (key_press){
             key_return = M_KEY;
             key_state  = KEY_STATE_0;
             key_time   = NULL;
             key_ctr = FALSE;
         }
         else
             key_state = KEY_STATE_2; 
           break;
      default:                                		// 特殊情况：key_state是其他值得情况，清零key_state。这种情况一般出现在没有初始化key_state，第一次执行这个函数的时候
            key_state = KEY_STATE_0;
           break;
    }

    return key_return;                          	// 返回按键值
} 

unsigned char key_read(void)
{
    static unsigned char key_state1 = 0, key_time1 = 0;
    unsigned char key_return, key_temp;
	
    key_return  = N_KEY;                          	// 清零 返回按键值
    key_temp    = key_driver();                    	// 读取键值
    
    switch (key_state1){
    
        case KEY_STATE_0:                           // 按键状态0：等待有效按键（通过 key_driver 返回的有效按键值）
            if (key_temp == S_KEY){             	// 如果是[单击]，不马上返回单击按键值，先进入 按键状态1，判断是否有[双击]的可能
            
                key_time1 = 0;                      // 清零计时
                key_state1 = KEY_STATE_1;
            }
            else{                                    // 如果不是[单击]，直接返回按键值。这里的按键值可能是：[长按]，[无效按键]
                key_return = key_temp;               // 返回 按键值
            }
            break;
        case KEY_STATE_1:                       	 // 按键状态1：判定是否有[双击]
            if (key_temp == S_KEY){            		 // 有[单击]后，如果在 设定的时间间隔（KEY_INTERVAL*10ms=30*10ms=300ms） 内，再次有[单击]，则为[双击]，但是不马上返回 有效按键值为[双击]，先进入 按键状态2，判断是否有[三击]
            
                key_return = KEY_DOUBLE;             // 返回 有效按键：[双击]
                key_state1 = KEY_STATE_0;      		 // 返回 按键状态0，等待新的有效按键
            }
            else{                                	 // 有[单击]后，如果在 设定的时间间隔（KEY_INTERVAL*10ms=30*10ms=300ms）内，没有[单击]出现，则判定为 [单击]
            
                key_time1++;                    	 // 计数 时间间隔
                
                if (key_time1 >= KEY_INTERVAL){   	 // 超过 时间间隔
                    key_return = S_KEY;       		 // 返回有效按键：[单击]
                    key_state1 = KEY_STATE_0; 		 // 返回按键状态0，等待新的有效按键
                }
            }
            break;
        default:                                	 // 特殊情况：key_state是其他值得情况，清零key_state。这种情况一般出现在 没有初始化key_state，第一次执行这个函数的时候
            key_state1 = KEY_STATE_0;
            break;
    }
    
    return key_return;                        		 // 返回 按键值
}

unsigned char reset_key_driver(void) 
{     
    static unsigned char rst_key_state  = RST_KEY_STATE_0;      // 按键状态变量
    static unsigned int rst_key_time    = 0;           		    // 按键计时变量
    unsigned char rst_key_press;
	unsigned char rst_key_return        = N_KEY;  				// 清除 返回按键值
	
    rst_key_press = GetResetKeyState();             		    // 读取当前键值

    switch (rst_key_state){     
        case RST_KEY_STATE_0:                       			// 按键状态0：判断有无按键按下
            if (!rst_key_press){                    		    // 有按键按下
                rst_key_time = 0;                   		    // 清零时间间隔计数
                rst_key_state = RST_KEY_STATE_1;        		// 然后进入 按键状态1
            }        
            break;
        case RST_KEY_STATE_1:                       			// 按键状态1：软件消抖（确定按键是否有效，而不是误触）。按键有效的定义：按键持续按下超过设定的消抖时间。
            if (!rst_key_press){                     
                rst_key_time++;                     		    // 一次10ms
                if(rst_key_time >= RST_KEY_SHORT_TIME){ 		// 消抖时间为：SINGLE_KEY_TIME*10ms = 30ms;
                    rst_key_state = RST_KEY_STATE_2;    		// 如果按键时间超过 消抖时间，即判定为按下的按键有效。按键有效包括两种：单击或者长按，进入 按键状态2， 继续判定到底是那种有效按键
                }
				}    
            else 
				rst_key_state = RST_KEY_STATE_0;       			// 如果按键时间没有超过，判定为误触，按键无效，返回 按键状态0，继续等待按键
            break; 
        case RST_KEY_STATE_2:                       			// 按键状态2：判定按键有效的种类：是单击，还是长按
            if(rst_key_press){                      		    // 如果按键在设定的长按时间内释放，则判定为单击
                 rst_key_return = RST_S_KEY;            		// 返回有效按键值：单击
                 rst_key_state = RST_KEY_STATE_0;       		// 返回按键状态0，继续等待按键
            }
            else{
                rst_key_time++;                     
                if(rst_key_time >= RST_KEY_LONG_TIME){   		// 如果按键时间超过 设定的长按时间（LONG_KEY_TIME*10ms=200*10ms=2000ms）, 则判定为 长按
                    rst_key_return = RST_L_KEY;          		// 返回 有效键值值：长按
                    rst_key_state = RST_KEY_STATE_3;     		// 去状态3，等待按键释放
                }
            }
            break;
      case RST_KEY_STATE_3:                         
          if (rst_key_press){ 								    // 等待按键释放
              rst_key_state = RST_KEY_STATE_0;          		// 按键释放后，进入 按键状态0 ，进行下一次按键的判定
          }         
          break;
      default:                                				    // 特殊情况：key_state是其他值得情况，清零key_state。这种情况一般出现在没有初始化key_state，第一次执行这个函数的时候
            rst_key_state = RST_KEY_STATE_0;
            break;
    }

    return rst_key_return;                          	        // 返回按键值
} 


