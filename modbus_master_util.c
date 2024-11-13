/*
 * modbus_master_util.c
 *
 *  Created on: 19 июл. 2024 г.
 *      Author: Rafael Boltachev
 */


#include "modbus_master_util.h"


void modbus_master_send_read_t(uint8_t address, uint8_t function_code, uint16_t reference_number, uint16_t data)
{
  modbus_read_t mbr;

  create_modbus_read(&mbr, address, function_code, reference_number, data);

  modbus_master_send((uint8_t *)&mbr, sizeof(mbr));
}

void modbus_master_send_read_coils(uint8_t address, uint16_t reference_number, uint16_t count)
{
  modbus_master_send_read_t(address, MQF_READ_COIL_STATUS, reference_number, count);
}

void modbus_master_send_read_discrete_inputs(uint8_t address, uint16_t reference_number, uint16_t count)
{
  modbus_master_send_read_t(address, MQF_READ_DISCRETE_INPUTS, reference_number, count);
}

void modbus_master_send_read_holding_registers(uint8_t address, uint16_t reference_number, uint16_t count)
{
  modbus_master_send_read_t(address, MQF_READ_HOLDING_REGISTERS, reference_number, count);
}

void modbus_master_send_read_input_registers(uint8_t address, uint16_t reference_number, uint16_t count)
{
  modbus_master_send_read_t(address, MQF_READ_INPUT_REGISTERS, reference_number, count);
}

void modbus_master_send_write_coil(uint8_t address, uint16_t reference_number, uint8_t value)
{
  modbus_master_send_read_t(address, MQF_WRITE_FORCE_SINGLE_COIL, reference_number, value ? 0xFF00 : 0);
}

void modbus_master_send_write_register(uint8_t address, uint16_t reference_number, uint16_t value)
{
  modbus_master_send_read_t(address, MQF_WRITE_SINGLE_REGISTER, reference_number, value);
}

void modbus_master_send_write_coils(uint8_t address, uint16_t reference_number, uint16_t count, uint8_t * values)
{
  uint16_t rc;
  uint8_t buf[260];
  uint8_t * bytes;
  uint16_t i, byte_num, bit_num, bytes_count;

  if (!count) return;

  bytes = buf + sizeof(modbus_write_header_t);
  bytes_count = 1 + (count - 1) / 8;
  for (i = 0; i < bytes_count; ++i) bytes[i] = 0;



  for (i = 0; i < count; ++i) {
    byte_num = i / 8;
    bit_num = i - byte_num * 8;
    if (values[i]) bytes[byte_num] |= (1 << bit_num);
  }


  rc = create_modbus_write(buf, sizeof(buf),  address, MQF_WRITE_FORCE_MULTIPLE_COILS,  reference_number, count, bytes_count, bytes);
  modbus_master_send(buf, rc);
}


void modbus_master_send_write_registers(uint8_t address, uint16_t reference_number, uint16_t count, uint16_t * registers)
{

  uint16_t rc;
  uint8_t buf[260];
  uint16_t * regs;
  uint16_t i;

  regs = (uint16_t *)(buf + sizeof(modbus_write_header_t));

  for (i = 0; i < count; ++i) regs[i] = HTONS(registers[i]);


  rc = create_modbus_write(buf, sizeof(buf), address, MQF_WRITE_PRESET_MULTIPLI_REGISTERS,  reference_number, count, count * 2, (uint8_t *)regs);
  modbus_master_send(buf, rc);
}





void modbus_master_recv(uint8_t * data, uint16_t data_len)
{

  modbus_error_t * mbe;
  modbus_response_read_header_t * mrh;
  modbus_read_t * mbr;


  uint16_t reference_number;
  uint16_t count;
  uint16_t i;
  uint16_t * preg, * preg2;
  uint8_t * pbyte;
  uint16_t byte_num, bit_num;

  uint8_t buf[260 * 8];


  if (data[1] & 0x80) {
	  mbe = parse_modbus_error(data, data_len);
	  if (!mbe) {
		  modbus_master_recv_error_crc();
		  return;
	  }
	  modbus_master_recv_error(mbe->address, mbe->function_code & 0x0F, mbe->error_code);
	  return;
  }




  switch (data[1]) {

    case MQF_READ_COIL_STATUS:

    	mrh = parse_modbus_response_read(data, data_len);
    	if (!mrh) {
    		modbus_master_recv_error_crc();
    		return;
    	}

    	pbyte = data + sizeof(modbus_response_read_header_t);

    	i = 0;
    	for (byte_num = 0; byte_num < mrh->byte_count; ++byte_num) {
        	for (bit_num = 0; bit_num < 8; ++bit_num) {
        		buf[i] = GETBIT(pbyte[byte_num], bit_num);
        		i++;
        	}
    	}


       modbus_master_recv_read_coils(mrh->address, mrh->byte_count * 8, buf);

    break;

    case MQF_READ_DISCRETE_INPUTS:

    	mrh = parse_modbus_response_read(data, data_len);
    	if (!mrh) {
    		modbus_master_recv_error_crc();
    		return;
    	}

    	pbyte = data + sizeof(modbus_response_read_header_t);

    	i = 0;
    	for (byte_num = 0; byte_num < mrh->byte_count; ++byte_num) {
        	for (bit_num = 0; bit_num < 8; ++bit_num) {
                 buf[i] = GETBIT(pbyte[byte_num], bit_num);
        	     i++;
        	}
    	}


       modbus_master_recv_read_discrete_inputs(mrh->address, mrh->byte_count * 8, buf);

    break;

    case MQF_WRITE_FORCE_SINGLE_COIL:

      mbr = parse_modbus_read(data, data_len);
      if (!mbr) {
		  modbus_master_recv_error_crc();
    	  return;
      }
      reference_number = HTONS(mbr->reference_number);
      count = HTONS(mbr->data);
      modbus_master_recv_write_coil(mbr->address, reference_number, count ? 1: 0);
    break;

   case MQF_WRITE_FORCE_MULTIPLE_COILS:

	  mbr = parse_modbus_read(data, data_len);
	  if (!mbr) {
		  modbus_master_recv_error_crc();
		  return;
	  }
	  reference_number = HTONS(mbr->reference_number);
	  count = HTONS(mbr->data);
      modbus_master_recv_write_coils(mbr->address, reference_number, count);

  break;


  case MQF_WRITE_SINGLE_REGISTER:

    mbr = parse_modbus_read(data, data_len);
    if (!mbr) {
 	    modbus_master_recv_error_crc();
    	return;
    }
    reference_number = HTONS(mbr->reference_number);
    count = HTONS(mbr->data);
    modbus_master_recv_write_register(mbr->address, reference_number, count);

  break;

  case MQF_READ_INPUT_REGISTERS:

  	mrh = parse_modbus_response_read(data, data_len);
  	if (!mrh) {
  		modbus_master_recv_error_crc();
  		return;
  	}

  	preg = (uint16_t *)(data + sizeof(modbus_response_read_header_t));
  	preg2 = (uint16_t *)buf;

  	count = mrh->byte_count / 2;
	for (i = 0; i < count; ++i) preg2[i] = HTONS(preg[i]);

	modbus_master_recv_read_input_registers(mrh->address, count, preg2);
  break;

  case MQF_READ_HOLDING_REGISTERS:

  	mrh = parse_modbus_response_read(data, data_len);
  	if (!mrh) {
  		modbus_master_recv_error_crc();
  		return;
  	}

  	preg = (uint16_t *)(data + sizeof(modbus_response_read_header_t));
    preg2 = (uint16_t *)buf;

    count = mrh->byte_count / 2;
	for (i = 0; i < count; ++i) preg2[i] = HTONS(preg[i]);

	modbus_master_recv_read_holding_registers(mrh->address, count, preg2);
  break;
  case MQF_WRITE_PRESET_MULTIPLI_REGISTERS:

	  mbr = parse_modbus_read(data, data_len);
      if (!mbr) {
    	  modbus_master_recv_error_crc();
    	  return;
      }

      reference_number = HTONS(mbr->reference_number);
      count = HTONS(mbr->data);

      modbus_master_recv_write_registers(mbr->address, reference_number, count);

  break;

  }


}
