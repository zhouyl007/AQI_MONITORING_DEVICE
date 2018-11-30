#ifndef _RING_BUF_H_
#define _RING_BUF_H_


/*环形缓冲区管理器*/
typedef struct{

     unsigned char *buf;    /*环形缓冲区        */
     unsigned int size;     /*环形缓冲区        */
     unsigned int front;    /*头指针            */
     unsigned int rear;     /*尾指针            */
}ring_buf_t;

/*-------------------------外部接口声明----------------------------*/

int ring_buf_create(ring_buf_t *r,unsigned char *buf,unsigned int size);
void ring_buf_clr(ring_buf_t *r);
unsigned int ring_buf_len(ring_buf_t *r);
unsigned int ring_buf_put(ring_buf_t *r,unsigned char *buf,unsigned int len);
unsigned int ring_buf_get(ring_buf_t *r,unsigned char *buf,unsigned int len);

#endif

