
//This class deals with accepting incoming connections to the command socket and spawning corresponding individual data sockets for each incoming connection
class portalSocketInterface : public hierarchicalDataflowBlock{
public:
	portalSocketInterface(int portnum, socketType in_socket_type, genericSDRInterface *in_sdr_int);
	void *connectionListenerThread();
	int initSocket(int portnum);

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0){};
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
private:
	socketType socket_type;
	genericSDRInterface *sdr_int;
	int socket_fid;
	pthread_t conn_listener;
};
