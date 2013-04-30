#include <portalSocketInterface.h>

static void *socketListenerProxy(void *in_ptr){
	static_cast<portalSocketInterface*>(in_ptr)->connectionListenerThread();
}

portalSocketInterface::portalSocketInterface(int portnum, socketType in_socket_type, genericSDRInterface *in_sdr_int) : hierarchicalDataflowBlock(2, 0){
	socket_type = in_socket_type;
	sdr_int = in_sdr_int;

	//Open up a socket, but first initialize
	int socket_fid = initSocket(portnum);

	//Start up a thread to listen for incoming connections
	pthread_create(&conn_listener, NULL, socketListenerProxy, (void*)this);
}

void genericSDRInterface::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){
	//Channel 0 = Command Channel
	if(local_down_channel == 0){
		//Commands should all come in as vectors of strings
		std::vector<std::string> *in_command_vector = (std::vector<std::string> *)(data);
		sdr_int->setSDRParameter(in_comand_vector[0], in_command_vector[1]);
	}

	//Channel 1 = Data Channel
	else if(local_down_channel == 1){
		//This is raw I/Q data or at least some sort of raw data + metadata stream.  Forward to derived class
		iqData *in_iq = (iqData *)(data);
		sdr_int->sendIQData(in_iq->data, in_iq->num_bytes, uid_to_channel[in_iq->channel_uid]);
	}
}

int portalSocketInterface::initSocket(int portnum){
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
	in_addr.sin_port = htons(portnum);//This just needs to be set to zero to get a randomly-assigned free port number!

	if(bind(ret_id, (struct sockaddr *) &in_addr, sizeof(in_addr)) < 0)
	{
		exit(1); //TODO: Maybe this shouldn't be so harsh...
	}

	listen(ret_id, 5);

	return ret_id;
}

void *portalSocketInterface::connectionListenerThread(){
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
			
		//Create two objects using this connection, one command connection and one data connection which is bound to a random port
		portalDataSocket *new_data_socket = new portalDataSocket(socket_type, sdr_int);
		int data_port_fid = new_data_socket->initSocket();
		new socketThread(datasock_fd, new_data_socket, &uplink_lock, false);
	}
}
