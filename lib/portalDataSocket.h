
class portalDataSocket{
public:
	portalDataSocket(socketType in_socket_type, genericSDRInterface *in_sdr_int);
	int initSocket(int portnum);
	void *listeningThread();
private:
	socketType socket_type;
	genericSDRInterface *sdr_int;
	int socket_fid;
	pthread_t conn_listener;
};
