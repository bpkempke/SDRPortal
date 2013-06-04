
class portalDataSocket : public hierarchicalDataflowBlock{
public:
	portalDataSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_int);
	~portalDataSocket();

	int getPortNum();
	void setUID(int in_uid);

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
private:
	int uid;
	genericSocketInterface *socket_int;
	genericSDRInterface *sdr_int;
};
