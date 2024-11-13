#ifndef modbus_utilH
#define modbus_utilH

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

//чтение значений из нескольких регистров флагов   (Чтение DO)
#define MQF_READ_COIL_STATUS                    0x01
// чтение значений из нескольких дискретных входов  (Чтение DI)
#define MQF_READ_DISCRETE_INPUTS                0x02
//чтение значений из нескольких регистров хранения (Чтение AO)
#define MQF_READ_HOLDING_REGISTERS              0x03
//чтение значений из нескольких регистров ввода  (Чтение AI)
#define MQF_READ_INPUT_REGISTERS                0x04

//запись значения одного флага  (Запись одного DO)
#define MQF_WRITE_FORCE_SINGLE_COIL             0x05
// запись значения в один регистр хранения  (Запись одного AO)
#define MQF_WRITE_SINGLE_REGISTER               0x06
//запись значений в несколько регистров флагов  (Запись нескольких DO)
#define MQF_WRITE_FORCE_MULTIPLE_COILS          0x0F
//запись значений в несколько регистров хранения (Запись нескольких AO)
#define MQF_WRITE_PRESET_MULTIPLI_REGISTERS     0x10



//Принятый код функции не может быть обработан.
#define MBE_FUNCTION_NOT_AVIALIBLE      0x01
//Адрес данных, указанный в запросе, недоступен.
#define MBE_ADDR_NOT_AVIALIBLE          0x02
//Значение, содержащееся в поле данных запроса, является недопустимой величиной.
#define MBE_DATA_NOT_AVIALIBLE          0x03
//Невосстанавливаемая ошибка имела место, пока ведомое устройство пыталось выполнить затребованное действие.
#define MBE_NON_RENEWABLE               0x04
//Ведомое устройство приняло запрос и обрабатывает его, но это требует много времени. Этот ответ предохраняет ведущее устройство от генерации ошибки тайм-аута.
#define MBE_TIMEWAIT                    0x05
//Ведомое устройство занято обработкой команды. Ведущее устройство должно повторить сообщение позже, когда ведомое освободится.
#define MBE_BUSY                        0x06
//Ведомое устройство не может выполнить программную функцию, заданную в запросе. Этот код возвращается для неуспешного программного запроса, использующего функции с номерами 13 или 14. Ведущее устройство должно запросить диагностическую информацию или информацию об ошибках от ведомого.
#define MBE_FUNCTION_NOT_EXECUTE        0x07
//Ведомое устройство при чтении расширенной памяти обнаружило ошибку контроля четности.
#define MBE_CRC                         0x08


#ifndef HTONS
#define HTONS(a)			((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#endif


#ifndef HTONL
#define HTONL(a)			( (((a)>>24)&0xff) | (((a)>>8)&0xff00) |\
							(uint32_t)((((uint64_t)a)<<8) & 0xff0000) |\
							(uint32_t)((((uint64_t)a)<<24) & 0xff000000) )
#endif


#ifndef NULL
#define NULL				0
#endif


#pragma pack(push, 1)

//------------------------------------------------------------------------------
//------------------------------ MODBUS RTU----------------------------------------
//------------------------------------------------------------------------------
//запрос на чтение
typedef struct _modbus_read_t {
        uint8_t address;
        uint8_t function_code;
        uint16_t reference_number;
        uint16_t data;
        uint16_t crc;
} modbus_read_t;

//ответ на чтение
typedef struct _modbus_response_read_header_t {
        uint8_t address;
        uint8_t function_code;
        uint8_t byte_count;
} modbus_response_read_header_t;


//запрос на запись for 0x10
typedef struct _modbus_write_header_t {
       uint8_t address;
       uint8_t function_code;
       uint16_t reference_number;
       uint16_t count;
       uint8_t byte_count;
} modbus_write_header_t;

//ответ на запись
typedef modbus_read_t modbus_response_write_t;

/*
typedef struct _modbus_response_write_t {
        uint8_t address;
        uint8_t function_code;
        uint16_t reference_number;
        uint16_t data;
        uint16_t crc;
} modbus_response_write_t;
*/

//ошибки
typedef struct _modbus_error_t {
        uint8_t address;
        uint8_t function_code;
        uint8_t error_code;
        uint16_t crc;
} modbus_error_t;



#pragma pack(pop)


uint16_t modbus_crc(uint8_t * data, uint16_t length);



//парсить запрос на чтение
modbus_read_t * parse_modbus_read(uint8_t * data, uint16_t data_len);


//создать запрос на чтение
void create_modbus_read(modbus_read_t * pmodbus_read, uint8_t address, uint8_t function_code, uint16_t reference_number, uint16_t data);
//парсить ответ на чтение
modbus_response_read_header_t * parse_modbus_response_read(uint8_t * data, uint16_t data_len);
//создать ответ на чтение
uint16_t create_modbus_response_read(uint8_t * buf, uint16_t buf_size, uint8_t address, uint8_t function_code, uint8_t * data, uint8_t data_count);


//парсить запрос на запись
modbus_write_header_t * parse_modbus_write_header(uint8_t * data, uint16_t data_len);

//создать запрос на запись
uint16_t create_modbus_write(uint8_t * buf, uint16_t buf_size, uint8_t address, uint8_t function_code,  uint16_t reference_number, uint16_t count, uint8_t byte_count, uint8_t * data);


//парсить ответ на запись
//modbus_response_write_t * parse_modbus_response_write(uint8_t * data, uint16_t data_len);
#define parse_modbus_response_write 	parse_modbus_read
//создать ответ на запись
//void create_modbus_response_write(modbus_response_write_t * pmodbus_write,  uint8_t address, uint8_t function_code,  uint16_t reference_number, uint16_t  data);
#define create_modbus_response_write	create_modbus_read


//создать ошибку
void create_modbus_error(modbus_error_t * pmodbus_error, uint8_t address, uint8_t function_code, uint8_t error_code);
//парсить ошибку
modbus_error_t * parse_modbus_error(uint8_t * data, uint16_t data_len);

#ifdef __cplusplus
}
#endif


#endif
