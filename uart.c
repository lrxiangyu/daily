#include "uart.h"


#include "app_uart.h"
#define RX_BUF_MAX      50
#define HEAD_LEN        2
uint8_t my_rx_buf[RX_BUF_MAX];

static uint16_t rx_head = 0;
static uint16_t rx_tail = 0;
static uint8_t counter =0;

        
#define new_bytes(hd,tl)                    (hd>=tl?(hd-tl):(hd+RX_BUF_MAX-tl))
#define increase_ptr(p,n)  do{                                  \
                                p+=n;                             \
                             if(p >= my_rx_buf+RX_BUF_MAX)       \
                                 p -=RX_BUF_MAX;                          \
                            }while(0)                           
        
     
#define increase_len(pos,n) do{                                  \
                             if(pos+n >= RX_BUF_MAX)       \
                                 pos=pos+n-RX_BUF_MAX;     \
                             else                               \
                                 pos=pos+n;                         \
                            }while(0)                           


void send_string(uint8_t *ptr ,uint16_t len)
{
    while(len-->0)
    {
        while(app_uart_put(*ptr) != NRF_SUCCESS);
        ptr++;
    }
}
void rx_one_byte(uint8_t by)
{
    my_rx_buf[rx_head] = by;
    rx_head++;
    if(rx_head>=RX_BUF_MAX)
        rx_head = 0;
}
static void copy_data(uint8_t* dest,uint8_t* src,uint16_t len)
{
    while(len-- > 0)
    {
        *dest = *src;
        dest++;
        increase_ptr(src,1);
    }
}

bool check_head(void)
{
    uint8_t buf[HEAD_LEN];

    if(new_bytes(rx_head,rx_tail) >= 2)
    {
        copy_data(buf,&my_rx_buf[rx_tail],HEAD_LEN);

        if(buf[0] == 0XFF && buf[1] == 0XFE)
        {
            return 0;
        }
        increase_len(rx_tail,1);
    }
    return 1;
}



#define PACKAGE_LEN     5
bool check_checksum(void)
{
    if(new_bytes(rx_head,rx_tail) >= PACKAGE_LEN)
    {
        uint8_t buf[PACKAGE_LEN]={0};
        uint8_t checksum;
        copy_data(buf,&my_rx_buf[rx_tail],PACKAGE_LEN);
        //
        checksum = 0;
        for(uint16_t i = 0;i < PACKAGE_LEN - 1;i++)
		{
			checksum += buf[i];
		}
		checksum = 0x100 - (checksum & 0xff);
        if(checksum == buf[PACKAGE_LEN-1])
        {
            //right data
            
            send_string(buf,PACKAGE_LEN);
            increase_len(rx_tail,PACKAGE_LEN);
        }
        else
        {
            send_string(buf,HEAD_LEN);
            increase_len(rx_tail,HEAD_LEN);
        }
        
        return 0;
    }
    return 1;
}

uint8_t get_time_counter(void)
{
    return counter;
}
void clear_time_counter(void)
{
    counter = 0;
}

void rx_timer_counter(void)
{
    counter++;
}

void rx_data_parse(void)
{
    static uint8_t step = 0;
    
    switch(step)
    {
        case 0:
            if(check_head() == 0)
            {
                step++;
                //over time
                clear_time_counter();
            }
            break;
        case 1:
            if(get_time_counter() > 100)
            {
                step = 0;
                increase_len(rx_tail,2);
                send_string((uint8_t*)"over time",9);
            }
            else
            {
                if(check_checksum() == 0)
                {
                    step = 0;  
                }
            }
            break;
    }
    
}

void my_uart_init(void)
{

    rx_head = 0;
    rx_tail = 0; 
}

