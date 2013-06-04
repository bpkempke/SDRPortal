
class portalCommandSocket : public hierarchicalDataflowBlock{
public:
	portalCommandSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_int);
	~portalCommandSocket();

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
private:
	socketType socket_type;
	genericSocketInterface *socket_int;
	genericSDRInterface *sdr_int;
	int num_channels, cur_channel;
	map<portaDataSocket*,int> uid_map;
};
