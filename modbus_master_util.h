/*
 * modbus_master_util.h
 *
 *  Created on: 19 июл. 2024 г.
 *      Author: Rafael Boltachev
 */

#ifndef UTIL_MODBUS_MASTER_UTIL_H_
#define UTIL_MODBUS_MASTER_UTIL_H_

#include <stdint.h>
#include "modbus_util.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef GETBIT
#define GETBIT(v, n)	((0u == (v & (1<<n))) ? 0u : 1u)
#endif


//need to call this function when data arrives
void modbus_master_recv(uint8_t * data, uint16_t data_len);



void modbus_master_send_read_coils(uint8_t address, uint16_t reference_number, uint16_t count);
void modbus_master_send_read_discrete_inputs(uint8_t address, uint16_t reference_number, uint16_t count);
void modbus_master_send_read_holding_registers(uint8_t address, uint16_t reference_number, uint16_t count);
void modbus_master_send_read_input_registers(uint8_t address, uint16_t reference_number, uint16_t count);
void modbus_master_send_write_coil(uint8_t address, uint16_t reference_number, uint8_t value);
void modbus_master_send_write_register(uint8_t address, uint16_t reference_number, uint16_t value);
void modbus_master_send_write_coils(uint8_t address, uint16_t reference_number, uint16_t count, uint8_t * values);
void modbus_master_send_write_registers(uint8_t address, uint16_t reference_number, uint16_t count, uint16_t * registers);



//the functions located below must be written in your program
void modbus_master_send(uint8_t * data, uint16_t data_len);
void modbus_master_recv_error_crc(void);
void modbus_master_recv_error(uint8_t address, uint8_t function_code, uint8_t error_code);
void modbus_master_recv_read_coils(uint8_t address, uint16_t count, uint8_t * values);
void modbus_master_recv_read_discrete_inputs(uint8_t address, uint16_t count, uint8_t * values);
void modbus_master_recv_read_holding_registers(uint8_t address, uint16_t count, uint16_t * values);
void modbus_master_recv_read_input_registers(uint8_t address, uint16_t count, uint16_t * values);
void modbus_master_recv_write_coil(uint8_t address, uint16_t reference_number, uint8_t value);
void modbus_master_recv_write_register(uint8_t address, uint16_t reference_number, uint16_t value);
void modbus_master_recv_write_coils(uint8_t address, uint16_t reference_number, uint16_t count);
void modbus_master_recv_write_registers(uint8_t address, uint16_t reference_number, uint16_t count);




#ifdef __cplusplus
}
#endif


#endif /* UTIL_MODBUS_MASTER_UTIL_H_ */
