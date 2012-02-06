#include "conio.h"#include "BSController.h"
#include "Tserial_event.h"

short buffer[MAX_CHANNEL][MAX_BUFFER_SIZE];
int main(int argc, char* argv[])
{
     int  erreur;
     erreur = connect_bluesense("\\\\.\\COM3", 115200,256,1, SERIAL_PARITY_NONE, 8, false);
        if (!erreur){
            char c;
            do
            {
                c = getch();
                printf("_%c",c);
                if ( c == 'c'){
                   //connect
                   connect_bluesense("\\\\.\\COM3", 115200,256,1, SERIAL_PARITY_NONE, 8, false);
                }
                if ( c == 'd'){
                   //disconnect
                   disconnect_bluesense();
                }

                if ( c == 's'){
                   //Start Sampling
                   start_bluesense_sampling();
                }
                if ( c == 'p'){
                   //stop Sampling
                   stop_bluesense_sampling();
                }
                //sendDataWaitForConfirm(com,set_baud_pak,size);
                if ( c == 'w'){
                   //stop Sampling
                   wait_for_data_block(buffer,32);
                }
            }
            while (c!=13);

            disconnect_bluesense();

        }else{
               printf("ERROR : com->connect (%ld)\n",erreur);
                c = getch();
                return 0;
        }



}


