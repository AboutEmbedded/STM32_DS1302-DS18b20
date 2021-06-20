/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DS18B20_Temp_H
#define __DS18B20_Temp_H

//НЕОБХОДИМО ИЗМЕНЯТЬ
#define TimClock_us 5 //время 1 тика таймера в мкс
#define DS18_Port_Pin GPIOB,GPIO_PIN_11 //пин для DS18B20
#define Delay_750ms 425000 //т.к. есть задержки на исполнение кода (лучше изменить при частоте отличной от 36МГц) при 750000 возможен более длительный опрос

//
//Служебные
//

//Дефайны для DS18B20
#define NOID 0xCC //Пропустить идентификацию 0xcc
#define T_CONVERT  0x44 //Код измерения температуры 0x44
#define READ_DATA 0xBE //Передача байтов ведущему 0xbe

//Дефайны готовности температуры
#define Temp_not_update 0x00
#define Temp_update 0xff

//возможные состояния Buf_State
#define state_1W_epmty 0x00
#define state_1W_busy 0xff



//Структуры для приема и вывода пользовательских данных
struct DS_State_str
{
	//Служебные для таймера
	uint8_t stage;
	uint32_t Timer_for_ms;
	
	//Служебные для приема-передачи
	uint8_t Data_Rx;
	uint8_t Data_Tx;
	uint8_t Buf_State;
};
extern struct DS_State_str State_1W;


struct DS18B20str
{
	//Пользовательские
	uint8_t Flag_ready; //имеет два состояния 0xff - температура обновлена 0x00 - температура старая
	uint16_t Temp;
	float Temp_celsium;
};
extern struct DS18B20str DS18B20_Data;

void DS18B20_Tim();	//Получение температуры (функция вызывается в таймере)



#endif /* __DS18B20_Temp_H */
