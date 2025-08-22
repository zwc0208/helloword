#include "twi_i2c.h"
#include "sw_timer.h"


/**
 * @description i2c sda�ܽ�ģʽ
 * @param mode ģʽ
 * @return ��
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
 * @description i2c����
 * @return ��
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
 * @description i2c����
 * @return ��
 */
void twi_i2c_stop(void)
{
	TWI_I2C_SDA(0);
	TWI_I2C_SCL(1);
	delay_us(2);
	TWI_I2C_SDA(1);
}

/**
 * @description i2cӦ��
 * @return ��
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
 * @description i2c��Ӧ��
 * @return ��
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
 * @description i2c�ȴ�Ӧ��
 * @return �ɹ���ʧ��
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
 * @description i2cд�ֽ�
 * @param data ����
 * @return �ɹ���ʧ��
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
 * @description i2c���ֽ�
 * @param ack Ӧ��
 * @return �ɹ���ʧ��
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
 * @description i2c���Ĵ���
 * @param addr ��ַ
 * @param reg �Ĵ���
 * @param buff ����
 * @param len ����
 * @return �ɹ���ʧ��
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
 * @description i2c���Ĵ���
 * @param addr ��ַ
 * @param reg �Ĵ���
 * @param buff ����
 * @param len ����
 * @return �ɹ���ʧ��
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




