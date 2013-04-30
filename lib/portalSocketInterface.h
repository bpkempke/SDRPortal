
//This class deals with accepting incoming connections to the command socket and spawning corresponding individual data sockets for each incoming connection
class portalSocketInterface{
public:
	portalSocketInterface(int portnum, socketType in_socket_type, genericSDRInterface *in_sdr_int);
	void *connectionListenerThread();
	int initSocket(int portnum);
private:
	socketType socket_type;
	genericSDRInterface *sdr_int;
	int socket_fid;
	pthread_t conn_listener;
};
