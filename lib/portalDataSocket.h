
class portalDataSocket{
public:
	portalDataSocket(socketType in_socket_type, genericSDRInterface *in_sdr_int);
	int initSocket(int portnum);
private:
	void listeningThread();
	int socket_fid;
	pthread_t conn_listener;
};
