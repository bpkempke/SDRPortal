#include <portalCommandSocket.h>

portalCommandSocket::portalCommandSocket(socketType in_socket_type, genericSDRInterface *in_sdr_int) : hierarchicalDataflowBlock(1, 1){

}

void portalCommandSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0){

}

void portalCommandSocket::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){

}

