#include "main.h"
#include "DS18B20_Temp.h"

//переменные для работы по таймеру
uint16_t Time_us=0;	//отсчеты таймера
uint8_t cnt=0;

//Дефайны выбора функции внутри таймера
#define DS_select_non_action 0
#define DS_select_write 1
#define DS_select_read 2
#define DS_select_reset 3
uint8_t DS_select=DS_select_non_action;

//Дефайны для пинов
#define DS18_Write_1 HAL_GPIO_WritePin(DS18_Port_Pin,GPIO_PIN_SET)	//т.к. притяжка к VCC, то инверсия
#define DS18_Write_0 HAL_GPIO_WritePin(DS18_Port_Pin,GPIO_PIN_RESET)
#define DS18_Read HAL_GPIO_ReadPin(DS18_Port_Pin)

struct DS_State_str State_1W; //структура для отслеживания состояния 1wire
struct DS18B20str DS18B20_Data;//структура для DS18B20

extern TIM_HandleTypeDef htim3;

uint8_t DS_survey;//Переменная для временного хранения значения о наличии устройства на шине



//ФУНКЦИИ 1 WIRE

//
//Функция для передачи
//
uint8_t Write_bit(uint8_t Tx_Byte)
{
	switch (Time_us)
  {
  	case 0:
			DS18_Write_0;
  	break;
  	case 5:
			if ((Tx_Byte & (0x01<<cnt))>0) DS18_Write_1;
			else DS18_Write_0;
  	break;
		case 60:
			DS18_Write_1; //пауза (сбросится на следующем такте)
			Time_us=0;
			return 0xFF;
  	break;
  }
	Time_us=Time_us + TimClock_us;

};


void Write_Byte()
{
			if (Write_bit(State_1W.Data_Tx)==0xff) 
			{
				cnt++; //если считали 1 бит, то переходим к следующему
			}
			if(cnt==8)	//если приняли 8 бит
			{
				State_1W.Buf_State=state_1W_epmty;
				cnt=0;
				Time_us=0;
			}
};


//
//Функция для приема
//
uint8_t Read_bit()
{
	Time_us=Time_us + TimClock_us;
	switch (Time_us)
  {
  	case 5:
			DS18_Write_0;
  	break;
		case 10:
			DS18_Write_1;
  	break;
  	case 15:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10, GPIO_PIN_SET);
			if (DS18_Read>0) State_1W.Data_Rx|=(0x01<<cnt);
			else State_1W.Data_Rx|=0x00;
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10, GPIO_PIN_RESET);
  	break;
		case 60:
			DS18_Write_1; //пауза
  	break;
		case 125:
			Time_us=0;
			return 0xFF;
  	break;
  }
};

void Read_Byte()
{
			if (Read_bit()==0xff) 
			{
				cnt++;
			}
			if(cnt==8)
			{
				cnt=0;
				Time_us=0;
				DS_select=DS_select_non_action;
				State_1W.Buf_State=state_1W_epmty;
			}
};




//
//Функция для сброса
//
uint8_t DS18b20_Reset()
{
		switch (Time_us)
		{
			case 0:
				DS18_Write_0;
			break;
			case 480: //480
				DS18_Write_1;
			break;
			case 540: 					//480+60+00 мкс 540
				if (DS18_Read>0) DS_survey=0xFF;
				else DS_survey=0x00;
			break;
		}
		Time_us=Time_us + TimClock_us;
		
		if(Time_us>1020) //1020
		{
			Time_us=0;
			DS_select=DS_select_non_action; //
			State_1W.Buf_State=state_1W_epmty;
			return DS_survey;
		}

};




//ФУНКЦИЯ ДЛЯ DS18B20

//функция вызывается в таймере
void DS18B20_Tim()
{
		switch (State_1W.stage)
    {
			//Инициализация
    	case 0:	//сброс
					State_1W.Buf_State=state_1W_busy;	
					DS18b20_Reset();
    	break;
    	case 1://запрос без идентификатора
					State_1W.Buf_State=state_1W_busy;	
					State_1W.Data_Tx=NOID; //копируем байт
					Write_Byte();
    	break;
			case 2:	//старт определения температуры
					State_1W.Buf_State=state_1W_busy;	
					State_1W.Data_Tx=T_CONVERT; //копируем байт
					Write_Byte();
    	break;
			case 3:	//ожидание конвертации
				State_1W.Buf_State=state_1W_busy;
				State_1W.Timer_for_ms=State_1W.Timer_for_ms + TimClock_us;
				if(State_1W.Timer_for_ms>Delay_750ms) //760000мкс = 760мс
					{
						State_1W.Timer_for_ms=0;
						State_1W.Buf_State=state_1W_epmty;
					}
    	break;
			
			//начало приема					
			case 4: //сброс
				State_1W.Buf_State=state_1W_busy;	
				DS18b20_Reset();
    	break;	
			case 5: //запрос без идентификатора
				State_1W.Buf_State=state_1W_busy;	
				State_1W.Data_Tx=NOID; //копируем байт
				Write_Byte();
    	break;	
	
			case 6: //запрос чтения данных
				State_1W.Buf_State=state_1W_busy;	
				State_1W.Data_Tx=READ_DATA; //копируем байт
				Write_Byte();
    	break;	
				
			
			//сами данные		
			case 7: //считать младший байт	
				State_1W.Data_Rx=0; //зачищаем буфер		
			break;	
			case 8: //считать младший байт	
				State_1W.Buf_State=state_1W_busy;	
				Read_Byte();
				DS18B20_Data.Temp=State_1W.Data_Rx;
    	break;	

			case 9: //считать младший байт	
				State_1W.Data_Rx=0; //зачищаем буфер	
			break;
			case 10:  //считать старший байт
				State_1W.Buf_State=state_1W_busy;	
				Read_Byte();
    	break;	
			
			case 11: //Конвертирование
				DS18B20_Data.Temp=(State_1W.Data_Rx)<<8 | DS18B20_Data.Temp;
//			DS18B20_Data.Temp=0xfe6f;
			if(DS18B20_Data.Temp>0x800)			
			{
//				DS18B20_Data.Temp_celsium=(-(((~DS18B20_Data.Temp)+1)*0.0625));
				
				DS18B20_Data.Temp=~DS18B20_Data.Temp+1; //(((~DS18B20_Data.Temp)+1)*0.0625)
				DS18B20_Data.Temp_celsium=-(DS18B20_Data.Temp*0.0625);
			}
			else				DS18B20_Data.Temp_celsium=DS18B20_Data.Temp*0.0625;
			
				DS18B20_Data.Flag_ready=0xff;
    	break;	
			
    	default:
    		break;
    }
			if(State_1W.Buf_State==state_1W_epmty)
			{
				State_1W.stage++;
			}
			if(State_1W.stage==12) State_1W.stage=0;
};






