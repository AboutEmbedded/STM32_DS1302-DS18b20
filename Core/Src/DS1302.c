#include "main.h"
#include "DS1302.h"


struct Data_time_str Data_time;
struct Data_time_set_str Data_time_set;	

void Delay_us(uint32_t Time_us)
{
	while(Time_us)
	{
		Time_us--;
	}
}

void DS1302_WriteByte(uint8_t Byte_Tx)
{
		Delay_us(TimeDelay);
		for(uint16_t i=1;i<=0x80;i=i<<1)
		{
			
			if (Byte_Tx & i) HAL_GPIO_WritePin(DATA_Port_Pin, GPIO_PIN_SET);
			else HAL_GPIO_WritePin(DATA_Port_Pin, GPIO_PIN_RESET);

			//Формируем строб. импульс
			HAL_GPIO_WritePin(CLK_Port_Pin, GPIO_PIN_SET);
			Delay_us(TimeDelay);
			HAL_GPIO_WritePin(CLK_Port_Pin, GPIO_PIN_RESET);
			Delay_us(TimeDelay);
			
			//сброс пина
			HAL_GPIO_WritePin(DATA_Port_Pin, GPIO_PIN_RESET);	
		}
		HAL_GPIO_WritePin(DATA_Port_Pin, GPIO_PIN_SET);	//сбрасываем ножку (притяжка к + через резистор)
		Delay_us(TimeDelay);
}

uint8_t DS1302_ReadByte()
{
		uint8_t DataRx=0;
		for(uint16_t i=1;i<=0x80;i=i<<1)
		{		
			HAL_GPIO_WritePin(CLK_Port_Pin, GPIO_PIN_SET);
			
			if (HAL_GPIO_ReadPin(DATA_Port_Pin)) DataRx|=i;
			else DataRx|=0;
			Delay_us(TimeDelay);

			HAL_GPIO_WritePin(CLK_Port_Pin, GPIO_PIN_RESET);
			Delay_us(TimeDelay);
		}
		return DataRx;
}


void DS1302_write(uint8_t Reg_addr, uint8_t Data)
{
		HAL_GPIO_WritePin(Latch_Port_Pin, GPIO_PIN_SET);	//Защелка
		
			DS1302_WriteByte(Reg_addr);
			DS1302_WriteByte(Data);
	
		HAL_GPIO_WritePin(Latch_Port_Pin, GPIO_PIN_RESET); //Защелка
};

uint8_t DS1302_read(uint8_t Reg_addr)
{
		uint8_t DataRxB=0;
		HAL_GPIO_WritePin(Latch_Port_Pin, GPIO_PIN_SET);	//Защелка
		
			DS1302_WriteByte(Reg_addr);
			DataRxB=DS1302_ReadByte();


		HAL_GPIO_WritePin(Latch_Port_Pin, GPIO_PIN_RESET); //Защелка
	
		HAL_GPIO_WritePin(DATA_Port_Pin, GPIO_PIN_RESET);
		return DataRxB;
}


void Read_All_DS1302()
{
		uint8_t Data=0;
		Data=DS1302_read(0x81);
		Data_time.sec=((Data & 0x70)>>4)*10 + (Data & 0x0f);
		
		Data=DS1302_read(0x83);
		Data_time.min=((Data & 0x70)>>4)*10 + (Data & 0x0f);
		
		Data=DS1302_read(0x85); //Важно смотреть формат записи!!!
		if(Data & 0x80) Data_time.hour=(((Data & 0x20)>>5)*12) + ((Data & 0x10)>>4)*10+(Data & 0x0f);	//не проверил //12 часовой формат
		else 						Data_time.hour=((Data & 0x30)>>4)*10+(Data & 0x0f);																					//24 часовой формат
		
		Data=DS1302_read(0x87);
		Data_time.day=((Data & 0x30)>>4)*10+(Data & 0x0f);
		
		Data=DS1302_read(0x89);
		Data_time.month=((Data & 0x10)>>4)*10+(Data & 0x0f);
		
		Data=DS1302_read(0x8b);
		Data_time.day_of_week=Data & 0x07;
		
		Data=DS1302_read(0x8d);
		Data_time.year=((Data & 0xf0)>>4)*10+(Data & 0x0f);
}

void Write_All_DS1302()
{
	//Снятие защиты от записи
	DS1302_write(0x8E, 0x00);
	//Снятие защиты от записи
	DS1302_write(0x8E, 0x00);
	
	
		uint8_t Data=0;
		uint8_t buf=0;
//Секунды	
		buf=Data_time_set.sec % 10; //получаем остаток от деления
		Data=((Data_time_set.sec -buf) / 10)<<4;	//записываем десятки секунд
		Data|=buf; 																//записываем единицы секунд
		Data&=0x7f;
		DS1302_write(0x80, Data);
	
//Минуты	
	
		Data=0;
		buf=0;
		buf=Data_time_set.min % 10; //получаем остаток от деления
		Data=((Data_time_set.min -buf) / 10)<<4;	//записываем десятки минут
		Data|=buf; 																//записываем единицы минут
		DS1302_write(0x82, Data);

//Часы
		Data=0;
		buf=0;
		buf=Data_time_set.hour % 10; //получаем остаток от деления
		Data=((Data_time_set.hour -buf) / 10)<<4;	//записываем десятки часов
		Data|=buf; 																//записываем единицы часов
		Data&=0x3f;																//24 часовой формат
		DS1302_write(0x84, Data);

//Дни	
		Data=0;
		buf=0;
		buf=Data_time_set.day % 10; //получаем остаток от деления
		Data=((Data_time_set.day -buf) / 10)<<4;	//записываем десятки часов
		Data|=buf; 																//записываем единицы часов
		DS1302_write(0x86, Data);
		
//Месяцы	
		DS1302_write(0x88, Data_time_set.month);

//День недели	
		DS1302_write(0x8a, Data_time_set.day_of_week);

//Год
		Data=0;
		buf=0;
		buf=Data_time_set.year % 10; //получаем остаток от деления
		Data=((Data_time_set.year -buf) / 10)<<4;	//записываем десятки часов
		Data|=buf; 																//записываем единицы часов
		DS1302_write(0x8c, Data);
		
	//Установка защиты от записи
	DS1302_write(0x8E, 0x80);		
	//Установка защиты от записи
	DS1302_write(0x8E, 0x80);	
}



