#ifndef BSPROTOCOL_H
#define BSPROTOCOL_H

#define START_FLAG	(0xFC)
#define END_FLAG  	(0xFD)
#define ESCAPE_FLAG	(0xFE)
#define EXOR_PATTERN	(0x20)


unsigned short handle_rcved_packet(unsigned char* rcved_packet, int *size);
short int* read_data();
short int read_data_size();
void data_has_been_read();

// 5.2.35 reinitialize BluesenseAT via software
unsigned char* CS_INIT_BLUESENSEAT_REQ(unsigned char pak_n, int *size);
//  5.2.21 Request Baud rate
unsigned char* CS_WRITE_UART_BAUD_RATE_REQ (unsigned char pak_n, int baud_r, int *size);
// 5.2.22 confirm the successful Uart setting
unsigned short CS_WRITE_UART_BAUD_RATE_CFM (unsigned char* packet, int *size);
//  6.3.3 Establishment of a SPP to analog channel
unsigned char* CS_SPP_START_ANALOG_REQ (unsigned char pak_n ,unsigned char BT_address[8], unsigned char server_CH[2], int *size);
// 6.3.4 Confirm the connection establishment
unsigned short CS_SPP_START_ANALOG_CFM (unsigned char* packet, int *size);
//  6.3.5 choose between the different function mode
unsigned char* CS_SET_FUNCTION_MODE_REQ (unsigned char pak_n ,unsigned char mode, int *size);
// 6.3.6 Confirm
unsigned short CS_SET_FUNCTION_MODE_CFM (unsigned char* packet, int *size);
//  6.3.7 Configuration of the AD converter
unsigned char* CS_CONFIG_ADC_REQ (unsigned char pak_n ,int sample_rate,unsigned char no_of_channels, int *size);
// 6.3.8 Confirm of the ADC configuration
unsigned short CS_CONFIG_ADC_CFM (unsigned char* packet, int *size);
//  6.3.9 starting and stopping of the sampling
unsigned char* CS_START_STOP_ADC_REQ (unsigned char pak_n ,unsigned char start_or_stop, int *size);
// 6.3.10 Confirmation of ADC started or stopped
unsigned short CS_START_STOP_ADC_CFM (unsigned char* packet, int *size);



#endif BSPROTOCOL_H




