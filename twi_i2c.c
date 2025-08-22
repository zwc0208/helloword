#include "twi_i2c.h"
#include "sw_timer.h"


/**
 * @description i2c sda管脚模式
 * @param mode 模式
 * @return 无
 */
static void twi_i2c_set_sda_pin(uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = TWI_I2C_SDA_PIN;
	
	if(mode)
	{
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	}
	else
	{		
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	}	

	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(TWI_I2C_SDA_PORT, &GPIO_InitStruct);
}

/**
 * @description i2c启用
 * @return 无
 */
void twi_i2c_start(void)
{   
	TWI_I2C_SDA(1);
	TWI_I2C_SCL(1);
	delay_us(2);
	TWI_I2C_SDA(0);
	delay_us(2);	
	TWI_I2C_SCL(0);
}

/**
 * @description i2c禁用
 * @return 无
 */
void twi_i2c_stop(void)
{
	TWI_I2C_SDA(0);
	TWI_I2C_SCL(1);
	delay_us(2);
	TWI_I2C_SDA(1);
}

/**
 * @description i2c应答
 * @return 无
 */
static void twi_i2c_ack(void)
{ 
	TWI_I2C_SDA(0);
	delay_us(2);
	TWI_I2C_SCL(1);	
	delay_us(2);
	TWI_I2C_SCL(0);	
	delay_us(2);
	TWI_I2C_SDA(1);        
}

/**
 * @description i2c不应答
 * @return 无
 */
static void twi_i2c_nack(void)
{ 
	TWI_I2C_SDA(1);
	delay_us(2);	
	TWI_I2C_SCL(1);
	delay_us(2);
	TWI_I2C_SCL(0);
	delay_us(2);
	TWI_I2C_SDA(1);       
}

/**
 * @description i2c等待应答
 * @return 成功或失败
 */
static int32_t twi_i2c_wait_ack(void)
{
	int32_t cnt=0;
	
	TWI_I2C_SDA_OUTPUT();
	
	TWI_I2C_SDA(1);
	delay_us(2);
	TWI_I2C_SCL(1);
	delay_us(2);
	
	TWI_I2C_SDA_INPUT();
	
	while(TWI_I2C_READ())
	{
		if(cnt ++ > 250)
			return -1;
	}
	TWI_I2C_SCL(0);
	
	TWI_I2C_SDA_OUTPUT();
	delay_us(2);
	
	return 0; 
} 

/**
 * @description i2c写字节
 * @param data 数据
 * @return 成功或失败
 */
int32_t twi_i2c_write_byte(uint8_t data)
{
	TWI_I2C_SDA_OUTPUT();
	
	TWI_I2C_SCL(0);
	
	for(int i = 0;i < 8;i ++)
	{
		if(data & 0x80)	TWI_I2C_SDA(1);
		else     				TWI_I2C_SDA(0);
		data<<=1;

		delay_us(2);
		TWI_I2C_SCL(1);
		delay_us(2);
		TWI_I2C_SCL(0);
		delay_us(2);
	}
	if(twi_i2c_wait_ack() == -1)
		return -1;
	
	return 0;
}

/**
 * @description i2c读字节
 * @param ack 应答
 * @return 成功或失败
 */
uint8_t twi_i2c_read_byte(uint8_t ack) 
{
	uint8_t data = 0;
	
	TWI_I2C_SDA_INPUT();
	
	for(int i = 0;i < 8;i ++)
	{
		TWI_I2C_SCL(1);
		delay_us(2);
		data <<= 1;
		if(TWI_I2C_READ())data |= 0x01;
		else         			data &= ~0x01; 
		delay_us(2);
		TWI_I2C_SCL(0);
		delay_us(2);
	}
	
	TWI_I2C_SDA_OUTPUT();
	
	if(ack)twi_i2c_ack();
	else	 twi_i2c_nack();

	return data;
}

/**
 * @description i2c读寄存器
 * @param addr 地址
 * @param reg 寄存器
 * @param buff 数据
 * @param len 长度
 * @return 成功或失败
 */
int32_t twi_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *buff, uint16_t len) 				 
{
	int32_t ret = 0;
		
	twi_i2c_start();

	ret |= twi_i2c_write_byte(addr & 0xFE);
	
	ret |= twi_i2c_write_byte(reg);

	twi_i2c_start();
	
	ret |= twi_i2c_write_byte(addr);

	for(int32_t i = 0;i < len - 1;i++)
	{
		buff[i] = twi_i2c_read_byte(1);
	}
	
	buff[len - 1] = twi_i2c_read_byte(0);
	
	
	twi_i2c_stop();	
	
	return ret;
}

/**
 * @description i2c读寄存器
 * @param addr 地址
 * @param reg 寄存器
 * @param buff 数据
 * @param len 长度
 * @return 成功或失败
 */
int32_t twi_i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t *buff, uint16_t len) 				 
{ 
	int32_t ret = 0;	
	
	twi_i2c_start();
	
	ret = twi_i2c_write_byte(addr);
	
	ret |= twi_i2c_write_byte(reg);

	for(int32_t i = 0;i < len;i ++)
	{
		ret |= twi_i2c_write_byte(buff[i]);	
	}
	
	twi_i2c_stop();

	return ret;
}




