
#include "modbus_slave_util.h"


void modbus_slave_send_error(uint8_t modbus_slave_address, uint8_t function_code, uint8_t error_code)
{
  modbus_error_t mbe;

  create_modbus_error(&mbe, modbus_slave_address, 0x80 | function_code, error_code);
  modbus_slave_send((uint8_t *)&mbe, sizeof(mbe));
}




void modbus_slave_recv(uint8_t * data, uint16_t data_len)
{

  modbus_read_t * mbr;
  modbus_write_header_t * mwh;
  modbus_response_write_t mw;
  uint16_t reference_number;
  uint16_t cnt; 
  uint8_t buf[260], res;
  uint16_t i, j;
  uint16_t * regs;
  uint8_t * pbyte;
  uint16_t value, byte_num, bit_num;
  uint8_t on;


  switch (data[1]) {

    case MQF_READ_COIL_STATUS:
    case MQF_READ_DISCRETE_INPUTS:
    
      mbr = parse_modbus_read(data, data_len);
      if (!mbr) {
          modbus_slave_send_error(data[0], mbr->function_code, MBE_CRC);
    	  return;
      }
    
      reference_number = HTONS(mbr->reference_number);
      cnt = HTONS(mbr->data);

      if (!cnt) { 
         modbus_slave_send_error(data[0], mbr->function_code, MBE_DATA_NOT_AVIALIBLE);
         return;
      }

      pbyte = buf + sizeof(modbus_response_read_header_t);
    
      for (byte_num = 0; byte_num < 1 + ((cnt - 1) / 8); ++byte_num) pbyte[byte_num] = 0;

      res = 0;
      j = 0;
      for (i = 0; i < cnt; ++i) {
        res = modbus_slave_read_coil(data[0], data[1], reference_number + i, &on);
        if (res) continue;
        byte_num = i / 8;
        bit_num = i - byte_num * 8;
        if (on) SETBIT(pbyte[byte_num], bit_num);
        j++;
      }


      if (!j) {
           modbus_slave_send_error(data[0], mbr->function_code, res);
           return;
      }
    
      cnt = create_modbus_response_read(buf, sizeof(buf), data[0], mbr->function_code, pbyte,  1 + ((j - 1) / 8));
      modbus_slave_send(buf, cnt);    
      break;
      
    case MQF_WRITE_FORCE_SINGLE_COIL:
      mbr = parse_modbus_read(data, data_len);
      if (!mbr) {
          modbus_slave_send_error(data[0], mbr->function_code, MBE_CRC);
    	  return;
      }

      reference_number = HTONS(mbr->reference_number);
      cnt = HTONS(mbr->data);

      res = modbus_slave_write_coil(data[0], data[1], reference_number, cnt ? 1 : 0);

      if (res) {
         modbus_slave_send_error(data[0], mbr->function_code, res);
         return;
      }
    
      modbus_slave_send(data, data_len); //ответ тот же    
    break;

   case MQF_WRITE_FORCE_MULTIPLE_COILS:   
     mwh = parse_modbus_write_header(data, data_len);
     if (!mwh) return;
   
     reference_number = HTONS(mwh->reference_number);
     cnt = HTONS(mwh->count);

     pbyte = (uint8_t *)(data + sizeof(modbus_write_header_t));

     res = 0;
     j = 0;
     for (i = 0; i < cnt; ++i) {
       byte_num = i / 8;
       bit_num = i - byte_num * 8;      
       res = modbus_slave_write_coil(data[0], data[1], reference_number + i, GETBIT(pbyte[i], bit_num));
       if (res) continue;
       j++;
     }    

     if (!j) {
        modbus_slave_send_error(data[0], mwh->function_code, res);
        return;
     }
     
     create_modbus_response_write(&mw, data[0], mwh->function_code, reference_number, j);
     modbus_slave_send((uint8_t *)&mw, sizeof(mw));
  break;  
    
  case MQF_WRITE_SINGLE_REGISTER:
    mbr = parse_modbus_read(data, data_len);
    if (!mbr) {
        modbus_slave_send_error(data[0], mbr->function_code, MBE_CRC);
        return;
    }
    
    reference_number = HTONS(mbr->reference_number);
    cnt = HTONS(mbr->data);
    
    res = modbus_slave_write_register(data[0], data[1], reference_number, cnt);
    if (res) {
       modbus_slave_send_error(data[0], mbr->function_code, res);
       return;
    }

    modbus_slave_send(data, data_len); //ответ тот же    
  break;

  case MQF_READ_INPUT_REGISTERS:
  case MQF_READ_HOLDING_REGISTERS:
    mbr = parse_modbus_read(data, data_len);
    if (!mbr) {
        modbus_slave_send_error(data[0], mbr->function_code, MBE_CRC);
        return;
    }
    
    reference_number = HTONS(mbr->reference_number);
    cnt = HTONS(mbr->data);

    regs = (uint16_t *)(buf + sizeof(modbus_response_read_header_t));

    for (i = 0; i < cnt; ++i) {
      res = modbus_slave_read_register(data[0], data[1], reference_number + i, &value);
      if (res) {
         modbus_slave_send_error(data[0], mbr->function_code, res);
         return;
      }
      regs[i] = HTONS(value);
    }
    

    cnt = create_modbus_response_read(buf, sizeof(buf), data[0], mbr->function_code, (uint8_t *)regs, cnt * 2);
    modbus_slave_send(buf, cnt);    
  break;

  case MQF_WRITE_PRESET_MULTIPLI_REGISTERS:
    mwh = parse_modbus_write_header(data, data_len);
    if (!mwh) {
        modbus_slave_send_error(data[0], mwh->function_code, MBE_CRC);
        return;
    }
   
    reference_number = HTONS(mwh->reference_number);
    cnt = HTONS(mwh->count);

    if (!cnt) { 
       modbus_slave_send_error(data[0], mwh->function_code, MBE_DATA_NOT_AVIALIBLE);
       return;
    }

    regs = (uint16_t *)(data + sizeof(modbus_write_header_t));

    res = 0;
    j = 0;
    for (i = 0; i < cnt; ++i) {
      res = modbus_slave_write_register(data[0], data[1], reference_number + i, HTONS(regs[i]));
      if (res) continue;
      j++;
    }

    if (!j) {
         modbus_slave_send_error(data[0], mwh->function_code, res);
         return;
    }
    
    create_modbus_response_write(&mw, data[0], mwh->function_code, reference_number, j);
    modbus_slave_send((uint8_t *)&mw, sizeof(mw));
  break;
  
  default :
      modbus_slave_send_error(data[0], data[1], MBE_FUNCTION_NOT_AVIALIBLE);
  }


}
