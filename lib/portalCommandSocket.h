
class portalCommandSocket{
public:
	portalCommandSocket(socketType in_socket_type, genericSDRInterface *in_sdr_int);
private:
	void listeningThread();
};
