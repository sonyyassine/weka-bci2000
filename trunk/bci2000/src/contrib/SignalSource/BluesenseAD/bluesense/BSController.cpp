

#ifdef __BORLANDC__
#pragma hdrstop             // borland specific
#include <condefs.h>
#pragma argsused
#endif

#include "conio.h"
#include "BSProtocol.h"
#include "BSController.h"
#include "Tserial_event.h"

unsigned char * recieved_packet = new unsigned char[1000];
int recieved_packet_size = 0;
unsigned char packet_number = 0;
int g_sample_rate = 0;
int g_no_of_channels = 0;
int g_block_size = MAX_BUFFER_SIZE;
short int channel_sample[MAX_CHANNEL][MAX_BUFFER_SIZE];
short int channel_end_index[MAX_CHANNEL];
short int channel_start_index[MAX_CHANNEL];

HANDLE packet_rcved_Event;
HANDLE block_is_ready_event;
HANDLE ghMutex;
Tserial_event *com;

bool is_stopped = false;
bool is_started = false;
bool disconnected = true;


/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void OnDataArrival(int size, char *buffer)
{
    if ((size>0) && (buffer!=0))
    {
        bool end_flag_rcved = false;

        //printf("%i \n" , recieved_packet_size);
//todo: Some codes should be added to handle data before start flag!
        int h;


        buffer[size] = 0;
        //printf("OnDataArrival: ");
        for (h=0; h < size; h++){
                //printf("%X ", (unsigned char) buffer[h]);
                recieved_packet[recieved_packet_size + h] = (unsigned char) buffer[h];
                recieved_packet_size ++;
                if ((unsigned char) buffer[h] == END_FLAG){
                     end_flag_rcved = true;
                }
        }
        //printf("\n");

        if (end_flag_rcved){
        short int* sample_data;
        int data_size;
        int end_indx;
        int start_indx;
        int current_data_size;
        int start_minus_one;
        int channelNo = 0;
           WaitForSingleObject(ghMutex,INFINITE); //exclusivly add to buffer block
                switch (handle_rcved_packet(recieved_packet, &recieved_packet_size)){
                  case 6315:
                        if (!is_started){
                                break;
                        }
                        //data packet

                        sample_data = read_data();
                        data_size = read_data_size();
                        for (int i = 0;i < data_size;i++){
                         if (channelNo >= g_no_of_channels){
                             channelNo = 0;
                         }
                         start_minus_one =  channel_start_index[channelNo] != 0 ? (channel_start_index[channelNo] - 1) : (MAX_BUFFER_SIZE - 1);


                         if (channel_end_index[channelNo] == start_minus_one ){ //buffer is full
                                printf("buffer is full \n");
                              break;//ignore data
                         }

                         channel_end_index[channelNo]++;
                         if (channel_end_index[channelNo] >= MAX_BUFFER_SIZE){
                            channel_end_index[channelNo] = 0;
                         }

                         //printf("DDDDDDD %i %i %i \n",g_no_of_channels,channelNo,channel_end_index[channelNo]);
                         channel_sample[channelNo][channel_end_index[channelNo]] = sample_data[i] ;

                         channelNo++;
                        }
                        data_has_been_read();

                        current_data_size =  channel_end_index[0] - channel_start_index[0];

                        if (  current_data_size < 0 ){
                              current_data_size = MAX_BUFFER_SIZE + current_data_size;
                        }

                        //printf("CCCCC %i %i %i \n",channel_end_index[0],channel_start_index[0],current_data_size);
                        //data is ready to be sent as a block!
                        //printf("g_block_size ! %i \n", g_block_size);
                        if ( current_data_size > g_block_size ){
                            SetEvent(block_is_ready_event);
                        }

                        SetEvent(packet_rcved_Event);
                        break;
                  default:
                        SetEvent(packet_rcved_Event);
                        break;

                }
            ReleaseMutex(ghMutex);
                recieved_packet_size = 0;
                //if (f) {
                //     SetEvent(packet_rcved_Event);
                //     f = false;
                //} else
                //f = true;
        }

    }
}

int wait_for_data_block(short buffer[MAX_CHANNEL][MAX_BUFFER_SIZE],int block_size){
   g_block_size = block_size;
   long status = 0;
   //there is no problem to check buffer size without synchronization,
   //because it may only get bigger and deadlock wont happen and data is available to be read
   int current_data_size =  channel_end_index[0] - channel_start_index[0];
   if (  current_data_size < 0 ){
         current_data_size = MAX_BUFFER_SIZE + current_data_size;
   }
   if ( current_data_size > g_block_size ){
    //this may be is not the best way, but its easiest!
    //by setting status to this value this method will not wait because the block is ready :)
         status =  WAIT_OBJECT_0;
   }else {
     // we should wait till the block is not ready yet!!
     status = WaitForSingleObject(block_is_ready_event,INFINITE);
   }
   //printf("WaitForSingleObject \n");
   if ( status == WAIT_OBJECT_0){
        WaitForSingleObject(ghMutex,INFINITE); //exclusivly read block

        ///always data is available we should not worry about the circular array!!!
        for (int i=0;i < g_no_of_channels;i++){
                //buffer[i] = new short[block_size];
                int start_indx = channel_start_index[i];
                for (int j=0;j < block_size;j++){
                    buffer[i][j] =  channel_sample[i][start_indx]-2047;
                    start_indx++;
                    if (start_indx == MAX_BUFFER_SIZE){
                           start_indx = 0;
                    }
                }
                channel_start_index[i] = start_indx;
        }
        ReleaseMutex(ghMutex);
   }
   g_block_size = MAX_BUFFER_SIZE;
   return 1;
   //channel_sample = new short[g_no_of_channels][g_block_size];
}

void SerialEventManager(t_uint32 object, t_uint32 event)
{
    char *buffer;
    int   size;
    Tserial_event *com;

    com = (Tserial_event *) object;
    if (com!=0)
    {
        switch(event)
        {
            case  SERIAL_CONNECTED  :
                                        //printf("Connected ! \n");
                                        break;
            case  SERIAL_DISCONNECTED  :
                                        printf("Disonnected ! \n");
                                        break;
            case  SERIAL_DATA_SENT  :
                                        //printf("Data sent ! \n");
                                        break;
            case  SERIAL_RING       :
                                        printf("DRING ! \n");
                                        break;
            case  SERIAL_CD_ON      :
                                        printf("Carrier Detected ! \n");
                                        break;
            case  SERIAL_CD_OFF     :
                                        printf("No more carrier ! \n");
                                        break;
            case  SERIAL_DATA_ARRIVAL  :
                                        size   = com->getDataInSize();
                                        buffer = com->getDataInBuffer();
                                        OnDataArrival(size, buffer);
                                        com->dataHasBeenRead();
                                        break;
        }
    }
}

bool sendDataWaitForConfirm(Tserial_event *com, unsigned char * packet, int size){
        int counter = 0;
        int no_of_send = 0;
        long status = 0L;
        recieved_packet_size = 0;
        com->sendData( packet , size);
        com->setRxSize(1);
        while (true){
            if (counter == NO_OF_WAITS){
                 recieved_packet_size = 0;
                 com->sendData( packet , size);
                 com->setRxSize(1);
                 counter = 0;
                 no_of_send++;
            }
            counter ++;
            status = WaitForSingleObject(packet_rcved_Event,100);
            if (status == WAIT_OBJECT_0){
                  packet_number++;
                  return true;
            }
            if (no_of_send == MAX_RESEND){
                  return false;
            }
        }
}


int connect_bluesense  (const char *port, int rate,int sample_rate, int no_of_channels, int parity, char ByteSize, bool modem_events){
    int            erreur;

    if (!disconnected){
        return 0;
    }
    for (int i=0;i< MAX_CHANNEL;i++){
        channel_end_index[i] = 0;
        channel_start_index[i] = 0;
    }

    com = new Tserial_event();
    if (com!=0)
    {

        com->setManager(SerialEventManager);
        erreur = com->connect(port, rate, parity, ByteSize, modem_events);
        if (!erreur)
        {
            packet_rcved_Event = CreateEvent(NULL,FALSE,FALSE,"packet_rcved_Event");
//todo all packets should be deleted after use!
            int size = 0;
            //confing SPP baud
            unsigned char * packet = CS_WRITE_UART_BAUD_RATE_REQ (packet_number, rate ,&size);
            sendDataWaitForConfirm(com,packet,size);

            //establishing connection
            unsigned char bt_add[8],bt_serv[2];
            bt_add[0]= 0xF2;  //0xF2; //BT LSB address
            bt_add[1]= 0x1C;  //0x1C;
            bt_add[2]= 0x10;  //0x10;
            bt_add[3]= 0x00;  //0x00;
            bt_add[4]= 0x96;  //0x96;
            bt_add[5]= 0x00;  //0x00;
            bt_add[6]= 0xA0;  //0xA0;
            bt_add[7]= 0x00;  //0x00;
            bt_serv[0]= 0x05; //server ch 5
            bt_serv[1]= 0x00; //0x00;
            packet = CS_SPP_START_ANALOG_REQ (packet_number, bt_add,bt_serv, &size);
            sendDataWaitForConfirm(com,packet,size);

            //confing Function mode
            packet = CS_SET_FUNCTION_MODE_REQ (packet_number, 0x02,&size);
            sendDataWaitForConfirm(com,packet,size);

            //ADC config
            g_sample_rate = sample_rate;
            g_no_of_channels = no_of_channels;
            packet = CS_CONFIG_ADC_REQ (packet_number, g_sample_rate, g_no_of_channels,&size);
            sendDataWaitForConfirm(com,packet,size);

            CloseHandle(packet_rcved_Event);
            disconnected = false;

        }
        else{
            return erreur;
        }
        // ------------------

    }
    return 0;
}

int start_bluesense_sampling(){
     if (!is_started && !disconnected){
        for (int i=0;i< MAX_CHANNEL;i++){
           channel_end_index[i] = 0;
           channel_start_index[i] = 0;
        }
        block_is_ready_event = CreateEvent(NULL,FALSE,FALSE,"block_is_ready_event");
         ghMutex = CreateMutex(
                NULL,              // default security attributes
                FALSE,             // initially not owned
                NULL);             // unnamed mutex
        is_started = true;
        is_stopped = false;
        packet_rcved_Event = CreateEvent(NULL,FALSE,FALSE,"packet_rcved_Event");
        int size = 0;
        unsigned char *packet = CS_START_STOP_ADC_REQ (packet_number, 0x01 ,&size);
        sendDataWaitForConfirm(com,packet,size);
     }
}

int stop_bluesense_sampling(){
     if (!is_stopped && !disconnected){
        int size = 0;
        is_stopped = true;
        is_started = false;
        unsigned char *packet = CS_START_STOP_ADC_REQ (packet_number, 0x00 ,&size);
        sendDataWaitForConfirm(com,packet,size);
        CloseHandle(packet_rcved_Event);
        CloseHandle(block_is_ready_event);
        CloseHandle(ghMutex);
     }
}

int disconnect_bluesense(){
        if (!disconnected){
                stop_bluesense_sampling();
                disconnected = true;
                com->disconnect();
                delete com;
                com = 0;
        }
}
