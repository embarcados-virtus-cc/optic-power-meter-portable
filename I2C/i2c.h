#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include "hardware/i2c.h"
#include "pico/types.h"
#include "pico/stdlib.h"

/* Barramento físico */
#define I2C_PORT i2c1
#define I2C_SDA  11
#define I2C_SCL  10

/*INICIALIZAÇÃO */
bool sfp_i2c_init(i2c_inst_t *i2c, uint sda, uint scl, uint baudrate);

/* Memory Access */
bool sfp_read_block(i2c_inst_t *i2c,uint8_t dev_addr,uint8_t start_offset,uint8_t *buffer,uint8_t length);


#endif
