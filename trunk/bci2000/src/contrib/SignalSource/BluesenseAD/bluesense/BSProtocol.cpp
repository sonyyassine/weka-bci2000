#include <stdio.h>
#include "BSProtocol.h"



unsigned int crc_word= 0xFFFF;
short int * bs_buffer;
short no_of_sample;


const unsigned short int crctttab[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


//*******************************************************************************/
unsigned short int update_crc(unsigned int crc, unsigned char c)
{
	int tmp;
	tmp=(crc>>8)^c;
	crc=(crc<<8)^crctttab[tmp];
	return (unsigned short int)crc;
}

void show_chars(char *buffer, int size)
{
    if ((size>0) && (buffer!=0))
    {
        buffer[size] = 0;
        printf("OnDataArrival: ");
        for (int hh=0; hh < size; hh++){
                printf("%X ",(unsigned char) buffer[hh]);
        }
        printf("\n");
    }
}


unsigned char* OCTET_STUFFING(unsigned char* origPack, int *size){
    int inx = 1;
    while(inx < *size -1){
          if ( (origPack[inx]==START_FLAG) |  (origPack[inx]==END_FLAG) | (origPack[inx]==ESCAPE_FLAG)){
                unsigned char *save = new unsigned char[*size + 1];
                int j = 0;
                for (j = 0; j < inx; j++ )
                     save[j] = origPack[j];
                save[j] = ESCAPE_FLAG;
                if (origPack[inx]==START_FLAG)  save[j+1] =  0xDC;
                if (origPack[inx]==END_FLAG)    save[j+1] =  0xDD;
                if (origPack[inx]==ESCAPE_FLAG) save[j+1] =  0xDE;
                inx++;
                for (j = inx; j < *size; j++ )
                     save[j+1] = origPack[j];
                save[j+1] = 0;
                delete [] origPack;
                origPack =  save;
                (*size)++;
          }
          inx++;
    }
    //printf ("Send: ");
    //show_chars(origPack,*size);
    return origPack;
}

unsigned char* UNDO_OCTET_STUFFING(unsigned char* origPack, int *size){
    int inx = 0;
   // printf ("Recieved: ");
   // show_chars(origPack,*size);
    while(inx < *size){
          if (origPack[inx]==ESCAPE_FLAG){

                unsigned char next_char;
                if (origPack[inx+1]== 0xDC) next_char =  START_FLAG;
                else if (origPack[inx+1]== 0xDD) next_char =  END_FLAG;
                else if (origPack[inx+1]== 0xDE) next_char =  ESCAPE_FLAG;
                else{
                   inx++;
                   continue;
                }

                origPack[inx] = next_char;

                for (int j = inx + 1; j < *size; j++ )
                     origPack[j] = origPack[j+1];
                (*size)--;
          }
          inx++;
    }
    return origPack;

}





// 5.2.21 CS_WRITE_UART_BAUD_RATE_REQ
unsigned char* CS_WRITE_UART_BAUD_RATE_REQ (unsigned char pak_n, int baud_r, int *size)
{

 unsigned char *packet = new unsigned char[100];
 *size = 9;
 // Init the baud-rate CS_WRITE_UART_BAUD_RATE_REQ
 packet[0]= START_FLAG;
 packet[1]= pak_n;
 packet[2]= 0x0B;
 packet[3]= 0x07;

 if (baud_r==9600)        packet[4]= 0x60;
 else if (baud_r==38400)  packet[4]= 0x18;
 else if (baud_r==57600)  packet[4]= 0x10;
 else if (baud_r==115200) packet[4]= 0x08;

 packet[5]= 0x00;

 for (int i=1;i<6;i++) crc_word=update_crc(crc_word, packet[i]) ;

 packet[6]= (crc_word & 0x00FF);
 packet[7]= (crc_word & 0xFF00)>>8;

 packet[8]= END_FLAG;
 packet = OCTET_STUFFING(packet,size);
 crc_word=0xFFFF;//reinit CRC

 return packet;
}

// 5.2.22 CS_WRITE_UART_BAUD_RATE_CFM
unsigned short CS_WRITE_UART_BAUD_RATE_CFM (unsigned char* packet, int *size)
{
 printf("CS_WRITE_UART_BAUD_RATE_CFM \n");
 return 5222;
}

// 5.2.35  CS_INIT_BLUESENSEAT_REQ
// reinitialize BluesenseAT via software
unsigned char* CS_INIT_BLUESENSEAT_REQ(unsigned char pak_n, int *size){

  unsigned char *packet = new unsigned char[100];

  *size = 7;
  packet[0]= START_FLAG;
  packet[1]= pak_n;
  packet[2]= 0x11;
  packet[3]= 0x07;

  for (int i=1;i<4;i++) crc_word=update_crc(crc_word, packet[i]) ;

  packet[4]= (unsigned char)(crc_word & 0x00FF);
  packet[5]= (unsigned char)(crc_word & 0xFF00)>>8;

  packet[6]= END_FLAG;
  packet = OCTET_STUFFING(packet,size);

  crc_word=0xFFFF;//reinit CRC

  return packet;

}


//  6.3.3 Establishment of a SPP to analog channel
unsigned char* CS_SPP_START_ANALOG_REQ (unsigned char pak_n, unsigned char BT_address[8], unsigned char server_CH[2], int *size){
  unsigned char *packet = new unsigned char[100];

  *size = 17;
  packet[0]= START_FLAG;
  packet[1]= pak_n;
  packet[2]= 0x41;
  packet[3]= 0x07;
  packet[4]= BT_address[0] ; //0xF2; //BT LSB address
  packet[5]= BT_address[1] ; //0x1C;
  packet[6]= BT_address[2] ; //0x10;
  packet[7]= BT_address[3] ; //0x00;
  packet[8]= BT_address[4] ; //0x96;
  packet[9]= BT_address[5] ; //0x00;
  packet[10]= BT_address[6] ; //0xA0;
  packet[11]= BT_address[7] ; //0x00;
  packet[12]= server_CH[0] ; //0x05; //server ch 5
  packet[13]= server_CH[1] ; //0x00;

  for (int i=1;i<14;i++) crc_word=update_crc(crc_word, packet[i]) ;

  packet[14]= (unsigned char)(crc_word & 0x00FF);
  packet[15]= (unsigned char)(crc_word & 0xFF00)>>8;

  packet[16]= END_FLAG;
  packet = OCTET_STUFFING(packet,size);

  crc_word=0xFFFF;//reinit CRC

  return packet;
}


// 6.3.4 Confirm the connection establishment
unsigned short CS_SPP_START_ANALOG_CFM (unsigned char* packet, int *size){
        printf("CS_SPP_START_ANALOG_CFM \n");
        return 634;
}


//  6.3.5 choose between the different function mode
unsigned char* CS_SET_FUNCTION_MODE_REQ (unsigned char pak_n ,unsigned char mode, int *size){
       unsigned char *packet = new unsigned char[100];
       *size = 8;
       packet[0]= START_FLAG;
       packet[1]= pak_n;
       packet[2]= 0x42;
       packet[3]= 0x07;
       packet[4]= mode;  //Set as ADC mode
       for (int i=1;i<5;i++) crc_word=update_crc(crc_word, packet[i]) ;
       packet[5]= (unsigned char)(crc_word & 0x00FF);
       packet[6]= (unsigned char)(crc_word & 0xFF00)>>8;
       packet[7]= END_FLAG;
       packet = OCTET_STUFFING(packet,size);
       crc_word=0xFFFF; // reinizialize the crc value.
       return packet;
}

// 6.3.6 Confirm
unsigned short CS_SET_FUNCTION_MODE_CFM (unsigned char* packet, int *size){
        printf("CS_SET_FUNCTION_MODE_CFM \n");
        return 636;
}

//  6.3.7 Configuration of the AD converter
unsigned char* CS_CONFIG_ADC_REQ (unsigned char pak_n ,int sample_rate,unsigned char no_of_channels, int *size){
       unsigned char *packet = new unsigned char[100];
       *size = 10;
       packet[0]= START_FLAG;
       packet[1]= pak_n;
       packet[2]= 0x43;
       packet[3]= 0x07;
       // set the sample freq.
       packet[4]= (sample_rate & 0x00FF) ;
       packet[5]= (sample_rate & 0xFF00)>>8 ;

       // Set the n_of channels:
       if      (no_of_channels==1)  packet[6]=0x01 ;
       else if (no_of_channels==2)  packet[6]=0x03 ;
       else if (no_of_channels==3)  packet[6]=0x07 ;
       else if (no_of_channels==4)  packet[6]=0x0F ;
       else if (no_of_channels==5)  packet[6]=0x1F ;
       else if (no_of_channels==6)  packet[6]=0x3F ;
       else if (no_of_channels==7)  packet[6]=0x7F ;
       else if (no_of_channels==8)  packet[6]=0xFF ;

       for (int i=1;i<7;i++) crc_word=update_crc(crc_word, packet[i]) ;

       packet[7]= (unsigned char)(crc_word & 0x00FF);
       packet[8]= (unsigned char)(crc_word & 0xFF00)>>8;
       packet[9]= END_FLAG;
       packet = OCTET_STUFFING(packet,size);
       crc_word=0xFFFF; // reinizialize the crc value.
       return packet;

}


// 6.3.8 Confirm of the ADC configuration
unsigned short CS_CONFIG_ADC_CFM (unsigned char* packet, int *size){
        printf("CS_CONFIG_ADC_CFM \n");
        return 638;
}

//  6.3.9 starting and stopping of the sampling
unsigned char* CS_START_STOP_ADC_REQ (unsigned char pak_n ,unsigned char start_or_stop, int *size){
       unsigned char *packet = new unsigned char[100];
       *size = 8;
       packet[0]= START_FLAG;
       packet[1]= pak_n;
       packet[2]= 0x44;
       packet[3]= 0x07;
       packet[4]= start_or_stop ; //0x01 strat , 0x00 stop

       for (int i=1;i<5;i++) crc_word=update_crc(crc_word, packet[i]) ;

       packet[5]= (unsigned char)(crc_word & 0x00FF);
       packet[6]= (unsigned char)(crc_word & 0xFF00)>>8;
       packet[7]= END_FLAG;
       packet = OCTET_STUFFING(packet,size);
       crc_word=0xFFFF; // reinizialize the crc value.
       return packet;

}
// 6.3.10 Confirmation of ADC started or stopped
unsigned short CS_START_STOP_ADC_CFM (unsigned char* packet, int *size){
        printf("CS_START_STOP_ADC_CFM \n");
        return 6310;
}

// 6.3.15 Packet with sample value
unsigned short CS_ADC_DATA (unsigned char* packet, int *size){
        printf("CS_ADC_DATA \n");
        int No_Of_dataBytes = packet[6];
        No_Of_dataBytes << 8 ;
        No_Of_dataBytes +=  packet[5];
        //printf("No_Of_dataBytes %i \n",No_Of_dataBytes);
        no_of_sample =    No_Of_dataBytes/2;
        bs_buffer = new short [no_of_sample] ;
        short int sample_data = 0;
        for (int i=0; i < No_Of_dataBytes; i+=2){
            sample_data =  packet[7+i+1] * 32;
            sample_data += packet[7+i];
            //printf("%i ",sample_data);
            bs_buffer[i/2] =  sample_data;
        }

        //printf("\n");

        return 6315;
}

short int* read_data(){
        return bs_buffer;
}
short int read_data_size(){
        return no_of_sample;
}

void data_has_been_read(){
         delete []bs_buffer;
}


// 4.1.3 Cpmmand Reject
unsigned short COMMAND_REJECT (unsigned char* packet, int *size){
        printf("COMMAND_REJECT! Packet Number %X \n",packet[4]);
        return 413;
}

unsigned short handle_rcved_packet(unsigned char* rcved_packet, int *size){
      unsigned short result = 0;

      rcved_packet = UNDO_OCTET_STUFFING(rcved_packet, size);
      
      if (rcved_packet[0] != START_FLAG) result = 0;
      else {
        switch (rcved_packet[3]){
          case 0x04 :
                switch (rcved_packet[2]){
                        case 0x00 : //Command: Reject
                                result = COMMAND_REJECT(rcved_packet,size);
                                break;
                        default :
                                result = 0;
                                break;
                }
                break;

          case 0x09 :
                switch (rcved_packet[2]){
                        case 0x0B : //CS_WRITE_UART_BAUD_RATE_CFM
                                result = CS_WRITE_UART_BAUD_RATE_CFM(rcved_packet,size);
                                break;
                        case 0x41 : //  CS_SPP_START_ANALOG_CFM
                                result = CS_SPP_START_ANALOG_CFM(rcved_packet,size);
                                break;
                        case 0x42 : //  CS_SET_FUNCTION_MODE_CFM
                                result = CS_SET_FUNCTION_MODE_CFM(rcved_packet,size);
                                break;
                        case 0x43 : //  CS_CONFIG_ADC_CFM
                                result = CS_CONFIG_ADC_CFM(rcved_packet,size);
                                break;
                        case 0x44: //  CS_START_STOP_ADC_CFM
                                result = CS_START_STOP_ADC_CFM(rcved_packet,size);
                                break;
                        case 0x47: //  CS_ADC_DATA
                                result = CS_ADC_DATA(rcved_packet,size);
                                break;

                        default :
                                result = 0;
                                break;
                }
                break;
          default :
                result = 0;
                break;
        }
      }
      return  result;
}
