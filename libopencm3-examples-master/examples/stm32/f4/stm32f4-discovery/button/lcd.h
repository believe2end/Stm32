#ifndef _LCD_H_
#define _LCD_H_


//reset---PA0
//cs---PA1
//rs---PA2
//sclk---PA3
//sid---PA4
#define reset GPIO0
#define cs    GPIO1
#define rs    GPIO2
#define sclk  GPIO3
#define sid   GPIO4
extern void transfer_command(int data1);
extern void transfer_data(int data1);
extern void initial_lcd();


#endif
