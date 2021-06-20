#ifndef __DS1302_H
#define __DS1302_H

#define CLK_Port_Pin GPIOB,GPIO_PIN_8
#define DATA_Port_Pin GPIOB,GPIO_PIN_7
#define Latch_Port_Pin GPIOB,GPIO_PIN_6

#define TimeDelay 100

struct Data_time_str
	{
		uint8_t sec;
		uint8_t min;
		uint8_t hour;
		uint8_t day;
		
		uint8_t month;
		uint8_t day_of_week;
		uint8_t year;
	};
extern struct Data_time_str Data_time;
	
	
struct Data_time_set_str
	{
		uint8_t sec;
		uint8_t min;
		uint8_t hour;
		uint8_t day;
		
		uint8_t month;
		uint8_t day_of_week;
		uint8_t year;
	};
extern struct Data_time_set_str Data_time_set;	
	



void Read_All_DS1302(void);	//Вызываем функцию и автоматом заполняется структура Data_time 
void Write_All_DS1302(void);//заполняем структуру Data_time_set и вызываем функцию записи

#endif /* __DS1302_H */
