
#include "modbus_slave_util.h"

#define MAX_REGISTERS 10
static uint16_t registers[MAX_REGISTERS] = {0};

#define MAX_COILS 10
static uint8_t coils[MAX_COILS] = {0};

void modbus_slave_send(uint8_t * data, uint16_t data_len)
{
  Serial.write(data, data_len);
}



uint8_t modbus_slave_write_register(uint8_t address, uint8_t function_code, uint16_t number, uint16_t value)
{

  if (number >= MAX_REGISTERS) return MBE_ADDR_NOT_AVIALIBLE;
  registers[number] = value;
  return 0;
}


uint8_t modbus_slave_read_register(uint8_t address, uint8_t function_code, uint16_t number, uint16_t * value)
{
  if (number >= MAX_REGISTERS) return MBE_ADDR_NOT_AVIALIBLE;  
  *value = registers[number];
  return 0;
}


uint8_t modbus_slave_read_coil(uint8_t address, uint8_t function_code, uint16_t number, uint8_t * value)
{
  if (number >= MAX_REGISTERS) return MBE_ADDR_NOT_AVIALIBLE;    
  *value = coils[number];
  return 0;
}  

uint8_t modbus_slave_write_coil(uint8_t address, uint8_t function_code, uint16_t number,  uint8_t value)
{
  if (number >= MAX_REGISTERS) return MBE_ADDR_NOT_AVIALIBLE;  
  coils[number] = value;  
  return 0;
}  




void setup() {
  Serial.begin(9600);
}


void loop() {
  
    unsigned char buf[260];
    int rc;
    
    if (Serial.available() < 1) return;
    rc = Serial.readBytes(buf, sizeof(buf));
    if (rc < 1) return;
    if (buf[0] == 1) { //if addres = 1
      modbus_slave_recv(buf, rc);  
    }      
}
