#include <windows.h>
#include <atlstr.h>
#include <list>


const unsigned char kPacketSize = 42;

const unsigned char kBufferSize = (kPacketSize * 2) - 1;

unsigned char buffer[kBufferSize];




int main() {

    std::list<int> mylist;
    std::list<int>::iterator it1;



    const unsigned long event_mask_to_set = EV_RXCHAR;
    const unsigned long kInputBufferSize = 2000;
    const unsigned long kOutputBufferSize = 2000;
    const CString com_port_name = _T("COM3");
    const long kRecvBufferSize = 2000;
    HANDLE com_port_handle;
    DCB dcb;
    COMSTAT com_status;
    unsigned long event_mask = 0;
    unsigned long errors = 0;
    unsigned char recv_buffer[kRecvBufferSize];
    unsigned long number_of_bytes_read = 0;

    com_port_handle = CreateFile(com_port_name,
        GENERIC_READ | GENERIC_WRITE,
        0,      //  must be opened with exclusive-access
        NULL,   //  default security attributes
        OPEN_EXISTING, //  must use OPEN_EXISTING
        FILE_ATTRIBUTE_NORMAL,      //  not overlapped I/O
        NULL); //  hTemplate must be NULL for comm devices


    //  Initialize the DCB structure.
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);


    dcb.BaudRate = CBR_38400;     //  baud rate
    dcb.ByteSize = 8;             //  data size, xmit and rcv
    dcb.Parity   = NOPARITY;      //  parity bit
    dcb.StopBits = ONESTOPBIT;    //  stop bit

    SetCommState(com_port_handle, &dcb);




    // Set mask on EV_RXCHAR event
    SetCommMask(com_port_handle, event_mask_to_set);

    // Set size of the receive and transmission queue
    SetupComm(com_port_handle,kInputBufferSize,kOutputBufferSize);

    // Purge the receive buffer
    PurgeComm(com_port_handle, PURGE_RXCLEAR);

    unsigned int counter = 0;

    while (counter <= 100) {
        WaitCommEvent(com_port_handle, &event_mask, NULL);
        if (event_mask == EV_RXCHAR) {
            ClearCommError(com_port_handle, &errors, &com_status);
            if (errors == 0) {
                if (com_status.cbInQue) {
                    //read from buffer
                    SecureZeroMemory(recv_buffer, kRecvBufferSize);
                    ReadFile(com_port_handle, recv_buffer, com_status.cbInQue, &number_of_bytes_read, NULL);
                    //print
                    for(int i = 0; i < com_status.cbInQue; i++ ) {
                        mylist.push_back (recv_buffer[i]);
                    }
                    it1 = mylist.begin();
                    it1 = mylist.erase (mylist.begin(),--(--(mylist.end())));
                    printf("%d\r\n", mylist.size());
                    //            PurgeComm(com_port_handle, PURGE_RXCLEAR);
                } else {
                    PurgeComm(com_port_handle, PURGE_RXCLEAR);
                }
            } else {
                PurgeComm(com_port_handle, PURGE_RXCLEAR);
            }
        } else {
            PurgeComm(com_port_handle, PURGE_RXCLEAR);
        }
    }












    //  read_tread_handle_ = CreateThread(NULL, 0, ReadFromComPortInThread, this, 0,
    //	  NULL);
    //  return kOK;



    getchar();
    return 0;
}
