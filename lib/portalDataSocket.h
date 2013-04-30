
class portalDataSocket : public hierarchicalDataflowBlock{
public:
	portalDataSocket(socketType in_socket_type, genericSDRInterface *in_sdr_int);
	int initSocket(int portnum);
	void *listeningThread();

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
private:
	socketType socket_type;
	genericSDRInterface *sdr_int;
	int socket_fid;
	pthread_t conn_listener;
};
