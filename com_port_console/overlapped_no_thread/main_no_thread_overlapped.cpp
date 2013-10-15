#include <windows.h>
#include <atlstr.h>






int main() {
  const unsigned long event_mask_to_set = EV_RXCHAR;
  const unsigned long kInputBufferSize = 2000;
  const unsigned long kOutputBufferSize = 2000;
  const CString com_port_name = _T("COM2");
  const long kRecvBufferSize = 2000;
  HANDLE com_port_handle;
  DCB dcb;
  COMSTAT com_status;
  unsigned long event_mask = 0;
  unsigned long errors = 0;
  unsigned char recv_buffer[kRecvBufferSize];
  unsigned long number_of_bytes_read = 0;
  unsigned long wait_result = 1;
  OVERLAPPED overlapped_read_structure;
  unsigned long number_of_bytes_transferred = 0;
  overlapped_read_structure.hEvent = CreateEvent(NULL, true, false, NULL);

  com_port_handle = CreateFile(com_port_name,
                               GENERIC_READ | GENERIC_WRITE,
                               0,      //  must be opened with exclusive-access
                               NULL,   //  default security attributes
                               OPEN_EXISTING, //  must use OPEN_EXISTING
                               FILE_FLAG_OVERLAPPED,      //  overlapped I/O
                               NULL); //  hTemplate must be NULL for comm devices


    //  Initialize the DCB structure.
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);


    dcb.BaudRate = CBR_9600;     //  baud rate
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


    long sum = 0;

    while (true) {
        WaitCommEvent(com_port_handle, &event_mask,  &overlapped_read_structure);
        wait_result = WaitForSingleObject(overlapped_read_structure.hEvent, INFINITE);
        if (wait_result == WAIT_OBJECT_0) {
          if (GetOverlappedResult(com_port_handle, &overlapped_read_structure, &number_of_bytes_transferred, true)) {
            if (event_mask == 1) {
              ClearCommError(com_port_handle, &errors, &com_status);
              if (errors == 0) {
                if (com_status.cbInQue) {
                  //read from buffer
                  SecureZeroMemory(recv_buffer, kRecvBufferSize);
                  overlapped_read_structure.Offset = 0;
                  overlapped_read_structure.OffsetHigh = 0;
                  ReadFile(com_port_handle, recv_buffer, com_status.cbInQue, NULL/*&number_of_bytes_read*/, &overlapped_read_structure);
                  //print
//                sum += com_status.cbInQue;
//                printf("%d ",sum);
                  for(int i = 0; i < com_status.cbInQue; i++ ) {
                    printf("%c",recv_buffer[i]);
                  }
//                  PurgeComm(com_port_handle, PURGE_RXCLEAR);
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

    }
    }











    //  read_tread_handle_ = CreateThread(NULL, 0, ReadFromComPortInThread, this, 0,
    //	  NULL);
    //  return kOK;



    getchar();
    return 0;
}

class CustomComPort {
    unsigned long __stdcall CustomComPort::ReadFromComPortInThread(void* p_param);
    void OnComPortReceiveData(){};
    HANDLE com_port_handle_;
    char* receive_buffer_;
    char* receive_buffer1_;
    char bytes_received_;
    int kReceiveBufferSize;
    int kReceiveBufferSize1;
    int buffer_position_;
};

unsigned long __stdcall CustomComPort::ReadFromComPortInThread(void* p_param) {
    CustomComPort* p_this = static_cast<CustomComPort*>(p_param);
    unsigned long event_mask = 0;
    _OVERLAPPED overlapped_read_structure;
    unsigned long number_of_bytes_transferred = 0;
    unsigned long errors = 0;
    COMSTAT com_status;
    overlapped_read_structure.hEvent = CreateEvent(NULL, true, false, NULL);
    unsigned long wait_result = 1;
    while (true) {
        WaitCommEvent(p_this->com_port_handle_, &event_mask, &overlapped_read_structure);
        wait_result = WaitForSingleObject(overlapped_read_structure.hEvent, INFINITE);
        if (wait_result == WAIT_OBJECT_0) {
            if (GetOverlappedResult(p_this->com_port_handle_, &overlapped_read_structure, &number_of_bytes_transferred, true)) {
                if (event_mask == 1) {
                    ClearCommError(p_this->com_port_handle_, &errors, &com_status);
                    if (errors == 0) {
                        overlapped_read_structure.Offset = 0;
                        overlapped_read_structure.OffsetHigh = 0;
                        if (com_status.cbInQue) {
                            memset(p_this->receive_buffer_, 0, p_this->kReceiveBufferSize);
                            ReadFile(p_this->com_port_handle_, p_this->receive_buffer_, com_status.cbInQue, NULL, &overlapped_read_structure);
                            p_this->bytes_received_ = com_status.cbInQue;
                            //			  if (com_status.cbInQue > 200) {
                            //				  p_this->OnComPortReceiveData();
                            //			  } 
                            //			  Sleep(500);
                            if (com_status.cbInQue >= p_this->kReceiveBufferSize1) { //Filling the buffer
                                memcpy(p_this->receive_buffer1_, p_this->receive_buffer_, p_this->kReceiveBufferSize1);
                                p_this->OnComPortReceiveData();
                                p_this->buffer_position_ = 0;
                            } else if (com_status.cbInQue < p_this->kReceiveBufferSize1) {
                                if ( (p_this->kReceiveBufferSize1 - p_this->buffer_position_) > com_status.cbInQue ) {
                                    memcpy(&(p_this->receive_buffer1_[p_this->buffer_position_]), p_this->receive_buffer_, com_status.cbInQue);
                                    p_this->buffer_position_ = p_this->buffer_position_ + com_status.cbInQue;
                                    if (p_this->buffer_position_ == p_this->kReceiveBufferSize1) {
                                        p_this->OnComPortReceiveData();
                                        p_this->buffer_position_ = 0;
                                    }
                                } else if ( (p_this->kReceiveBufferSize1 - p_this->buffer_position_) <= com_status.cbInQue ) {
                                    memcpy(&(p_this->receive_buffer1_[p_this->buffer_position_]), p_this->receive_buffer_, p_this->kReceiveBufferSize1 - p_this->buffer_position_);
                                    p_this->buffer_position_ = p_this->buffer_position_ + (p_this->kReceiveBufferSize1 - p_this->buffer_position_);
                                    if (p_this->buffer_position_ == p_this->kReceiveBufferSize1) {
                                        p_this->OnComPortReceiveData();
                                        p_this->buffer_position_ = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return EXIT_SUCCESS;
}