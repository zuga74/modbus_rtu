#include "modbus_util.h"
#include <string.h>

uint16_t modbus_crc(uint8_t * data, uint16_t length)
{
  uint16_t j;
  uint16_t reg_crc = 0xFFFF;

  while(length--)
  {
    reg_crc ^= *data++;
    for(j=0;j<8;j++)
    {
      if(reg_crc & 0x01) reg_crc=(reg_crc>>1) ^ 0xA001; // LSB(b0)=1
      else reg_crc=reg_crc>>1;
    }
  }
  return reg_crc;

}

// ----------- READ ----------


modbus_read_t * parse_modbus_read(uint8_t * data, uint16_t data_len)
{
	modbus_read_t * res;

	if (data_len < sizeof(modbus_read_t)) return NULL;

	res = (modbus_read_t *)data;

	return res->crc == modbus_crc(data, 6) ? res : NULL;
}





void create_modbus_read(modbus_read_t * pmodbus_read, uint8_t address, uint8_t function_code, uint16_t reference_number, uint16_t data)
{
	pmodbus_read->address = address;
	pmodbus_read->function_code = function_code;
	pmodbus_read->reference_number = HTONS(reference_number);
	pmodbus_read->data = HTONS(data);
	pmodbus_read->crc = modbus_crc((uint8_t *)pmodbus_read, 6);
}


uint16_t create_modbus_response_read(uint8_t * buf, uint16_t buf_size, uint8_t address, uint8_t function_code, uint8_t * data, uint8_t data_count)
{
	uint16_t * pcrc;
	modbus_response_read_header_t * pmbread;
	uint16_t len_without_crc, len_with_crc;

	len_without_crc = sizeof(modbus_response_read_header_t) + data_count;
	len_with_crc = len_without_crc + sizeof(uint16_t);

	if (buf_size < len_with_crc) return 0;

	pmbread = (modbus_response_read_header_t *)buf;
	pmbread->address = address;
	pmbread->function_code = function_code;
	pmbread->byte_count = data_count;
	if (buf + sizeof(modbus_response_read_header_t) != data) {
		memcpy(buf + sizeof(modbus_response_read_header_t), data, data_count);
	}
	pcrc = (uint16_t *)(buf + len_without_crc);
	*pcrc = modbus_crc(buf, len_without_crc);
	return len_with_crc;
}


modbus_response_read_header_t * parse_modbus_response_read(uint8_t * data, uint16_t data_len)
{
	uint16_t crc, crc_r;
	uint16_t len_without_crc, len_with_crc;
	modbus_response_read_header_t * pheader;

	if (data_len < sizeof(modbus_response_read_header_t) + sizeof(uint16_t)) return NULL;

	pheader = (modbus_response_read_header_t *)data;

	len_without_crc = sizeof(modbus_response_read_header_t) + pheader->byte_count;
	len_with_crc = len_without_crc + sizeof(uint16_t);

	if (len_with_crc < data_len) return NULL;

	crc = *(uint16_t *)(data + len_without_crc);

	crc_r = modbus_crc(data, len_without_crc);

   if (crc != crc_r) return NULL;

   return crc == crc_r ?  pheader : NULL;
}

// ----------- WRITE ----------

modbus_write_header_t * parse_modbus_write_header(uint8_t * data, uint16_t data_len)
{
	modbus_write_header_t * res;
	uint16_t len_without_crc, len_with_crc;
	uint16_t crc_r, * pcrc;

	if (data_len < sizeof(modbus_write_header_t)) return NULL;

	res = (modbus_write_header_t *)data;

	len_without_crc = sizeof(modbus_write_header_t) + res->byte_count;
	len_with_crc = len_without_crc + sizeof(uint16_t);

	if (data_len < len_with_crc) return NULL;

	pcrc = (uint16_t *)(data + len_without_crc);
	crc_r = modbus_crc(data, len_without_crc);

	return crc_r == *pcrc ? res : NULL;

}



uint16_t create_modbus_write(uint8_t * buf, uint16_t buf_size, uint8_t address, uint8_t function_code,  uint16_t reference_number, uint16_t count, uint8_t byte_count, uint8_t * data)
{
	uint16_t * pcrc;
	modbus_write_header_t * pmbwrite;
	uint16_t len_without_crc, len_with_crc;



	pmbwrite = (modbus_write_header_t *)buf;

	pmbwrite->address = address;
	pmbwrite->function_code = function_code;
	pmbwrite->reference_number = HTONS(reference_number);
	pmbwrite->count = HTONS(count);
	pmbwrite->byte_count = byte_count;

	len_without_crc = sizeof(modbus_write_header_t) + pmbwrite->byte_count;
	len_with_crc = len_without_crc + sizeof(uint16_t);

	if (buf_size < len_with_crc) return 0;

	if (buf + sizeof(modbus_write_header_t) != data) {
		memcpy(buf + sizeof(modbus_write_header_t), data, pmbwrite->byte_count);
	}

	pcrc = (uint16_t *)(buf + len_without_crc);
	*pcrc = modbus_crc(buf, len_without_crc);

	return len_with_crc;
}

/*
modbus_response_write_t * parse_modbus_response_write(uint8_t * data, uint16_t data_len)
{
	modbus_response_write_t * res;
	uint16_t crc_r;

	res = (modbus_response_write_t *)data;

	crc_r = modbus_crc(data, 6);

	return res->crc == crc_r ? res : NULL;
}


void create_modbus_response_write(modbus_response_write_t * pmodbus_write,  uint8_t address, uint8_t function_code,  uint16_t reference_number, uint16_t data)
{
	pmodbus_write->address = address;
	pmodbus_write->function_code = function_code;

	pmodbus_write->reference_number = HTONS(reference_number);
	pmodbus_write->data = HTONS(data);
	pmodbus_write->crc = modbus_crc((uint8_t *)pmodbus_write, 6);
}
*/

//------ ERROR ------------

void create_modbus_error(modbus_error_t * pmodbus_error, uint8_t address, uint8_t function_code, uint8_t error_code)
{
	pmodbus_error->address = address;
	pmodbus_error->function_code = function_code;
	pmodbus_error->error_code = error_code;
	pmodbus_error->crc = modbus_crc((uint8_t *)pmodbus_error, 3);
}


modbus_error_t * parse_modbus_error(uint8_t * data, uint16_t data_len)
{
  uint16_t crc_r;
  modbus_error_t * res;

  if (data_len < sizeof(modbus_error_t)) return NULL;

  res = (modbus_error_t *)data;

  crc_r = modbus_crc(data, 3);

  return res->crc == crc_r ? res : NULL;
}

