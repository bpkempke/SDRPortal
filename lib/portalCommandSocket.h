
class portalCommandSocket : public hierarchicalDataflowBlock{
public:
	portalCommandSocket(socketType in_socket_type, genericSDRInterface *in_sdr_int);

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
private:
	void listeningThread();
};
