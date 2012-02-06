#ifndef BSCONTROLLER_H
#define BSCONTROLLER_H

#define NO_OF_WAITS    10
#define MAX_RESEND     10
#define MAX_CHANNEL    8
#define MAX_BUFFER_SIZE    4000

int start_bluesense_sampling();
int stop_bluesense_sampling();
int disconnect_bluesense();
int connect_bluesense  (const char *port, int rate,int sample_rate, int no_of_channels, int parity, char ByteSize, bool modem_events);
int wait_for_data_block(short buffer[MAX_CHANNEL][MAX_BUFFER_SIZE],int block_size);

#endif BSPROTOCOL_H
