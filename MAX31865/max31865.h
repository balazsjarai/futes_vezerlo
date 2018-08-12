//#define SET_SPI_LS_OUTPUT		DDRE |=  (1<<PE2)
#define SET_SPI_MOSI_OUTPUT		DDRB |=  (1<<PINB2)
#define SET_SPI_SCK_OUTPUT		DDRB |=  (1<<PINB1)
#define SET_SPI_MISO_INPUT		DDRB &=~ (1<<PINB3)

#define SET_MAX1_CS_OUTPUT		DDRA |=  (1<<PINA7)
//#define SET_MAX2_CS_OUTPUT		DDRA |=  (1<<PA3)
//#define SET_MAX1_DR_INPUT		DDRA &=~ (1<<PA0)
//#define SET_MAX2_DR_INPUT		DDRA &=~ (1<<PA1)


#define SET_SPI_MOSI_HIGH		PORTB |=  (1<<PINB2)
#define SET_SPI_SCK_HIGH		PORTB |=  (1<<PINB1)
#define ENABLE_PULLUP_SPI_MISO	PORTB |=  (1<<PINB3)

//#define SET_SPI_LS_LOW			PORTE &=~  (1<<PE2)
#define SET_MAX1_CS_LOW			PORTA &=~  (1<<PINA7)
//#define SET_MAX2_CS_LOW			PORTA &=~  (1<<PA3)

//#define SET_SPI_LS_HIGH			PORTE |=  (1<<PE2)
#define SET_MAX1_CS_HIGH		PORTA |=  (1<<PINA7)
//#define SET_MAX2_CS_HIGH		PORTA |=  (1<<PA3)


#define READ_CONFIG						0x00
#define WRITE_CONFIG					0x80//0x80
#define READ_RTD_MSB					0x01
#define READ_RTD_LSB					0x02

#define READ_HIGH_FAULT_THRESHOLD_MSB	0x03
#define READ_HIGH_FAULT_THRESHOLD_LSB	0x04
#define READ_LOW_FAULT_THRESHOLD_MSB	0x05
#define READ_LOW_FAULT_THRESHOLD_LSB	0x06

#define WRITE_HIGH_FAULT_THRESHOLD_MSB	0x83
#define WRITE_HIGH_FAULT_THRESHOLD_LSB	0x84
#define WRITE_LOW_FAULT_THRESHOLD_MSB	0x85
#define WRITE_LOW_FAULT_THRESHOLD_LSB	0x86

#define READ_ERROR_STATE				0x07
#define DUMMYBYTE						0xFF




int8_t check_MAX31865(int8_t channel);

void init_max31865(int channel);
int16_t Read_MAX31865(int channel,int16_t referenz);
int16_t calc_temp_pt100(int16_t resistance);
void SPI_OFF();
void SPI_MAX31865();
