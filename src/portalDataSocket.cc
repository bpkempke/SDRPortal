#include <portalDataSocket.h>

static void *listeningThreadProxy(void *in_ptr){
	static_cast<portalDataSocket*>(in_ptr)->listeningThread();
}

portalDataSocket::portalDataSocket(socketType in_socket_type, genericSDRInterface *in_sdr_itn) : hierarchicalDataflowBlock(1, 1){
	socket_type = in_socket_type;
	sdr_int = in_sdr_int;

	//Open up a socket, but first initialize
	int socket_fid = initSocket(portnum);

	//Start up a thread to listen for incoming connections
	pthread_create(&conn_listener, NULL, listeningThreadProxy, (void*)this);

}

void portalDataSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0){

}

void portalDataSocket::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){

}

int portalDataSocket::initSocket(int portnum){
	struct sockaddr_in in_addr;

	int ret_id;

	//Open the socket descriptor
	ret_id = socket(AF_INET, SOCK_STREAM, 0);

	// set SO_REUSEADDR on a socket to true (1):
	int optval = 1;
	setsockopt(ret_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	//Set up socket-specific information, such as address, etc.
	memset((char *)&in_addr, 0, sizeof(sockaddr_in));
	in_addr.sin_family = AF_INET;
	in_addr.sin_addr.s_addr = INADDR_ANY;
	in_addr.sin_port = 0;//This just needs to be set to zero to get a randomly-assigned free port number!

	if(bind(ret_id, (struct sockaddr *) &in_addr, sizeof(in_addr)) < 0)
	{
		exit(1); //TODO: Maybe this shouldn't be so harsh...
	}

	listen(ret_id, 1);

	return ret_id;
}

void portalDataSocket::listeningThread(){
	/*
	 * void *connectionListenerThread()
	 *
	 *   This thread listens for incoming connection requests on the corresponding
	 *   socket and instantiates an associated object to deal with each new one.
	 */
	struct sockaddr_in data_cli;
	int datasock_fd;
	socklen_t data_cli_len;

	while(1){
		data_cli_len = sizeof(data_cli);
		datasock_fd = accept(socket_fid, (struct sockaddr *) &data_cli, &data_cli_len);

		//TODO: put some sort of error checking here....
		if(datasock_fd < 0)
			printf("ERROR on accept, socket_fid = %d\n", socket_fid);
			
		//Since we can only accept one connection at a time, the rest of the socket reading (which needs to be running inside of a thread) just goes here
		socketInterface socket_int(socket_type);
		socket_int.bind(datasock_fd);
		//TODO: This...
	}
}
