class paramData {
public:
	paramData(double in_data, int channel=0){scratch = new double(in_data); channel = in_channel;};
	paramData(int in_data, int channel=0){scratch = new int(in_data); channel = in_channel;};
	~paramData(){delete scratch;};

	//Accessor methods
	int getInt(){int ret_val; memcpy(&ret_val, scratch, sizeof(int)); return ret_val;};
	int getDouble(){double ret_val; memcpy(&ret_val, scratch, sizeof(double)); return ret_val;};
	int getChannel(){return channel;};
private:
	void *scratch;
	int channel;
};

template<class T>
struct paramAccessors {
	enum{DOUBLE, INT, VOID} arg_type;
	void (T::*setMethod)(paramData);
	paramData (T::*getMethod)();
	bool (T::*checkMethod)(paramData);
};

class genericSDRInterface {
public:
	void setSDRParameter(std::string name, std::string val);
	void sendIQData(void *data, int num_elements, int uid_port);
	int getRXPortUID(int rx_port);
	int getTXPortUID(int tx_port);
	int getGenericPortUID(int generic_port);
protected:
	virtual void setRXFreq(paramData in_param) = 0;
	virtual void setTXFreq(paramData in_param) = 0;
	virtual void setRXGain(paramData in_param) = 0;
	virtual void setTXGain(paramData in_param) = 0;
	virtual void setRXRate(paramData in_param) = 0;
	virtual void setTXRate(paramData in_param) = 0;
	virtual paramData getRXFreq() = 0;
	virtual paramData getTXFreq() = 0;
	virtual paramData getRXGain() = 0;
	virtual paramData getTXGain() = 0;
	virtual paramData getRXRate() = 0;
	virtual paramData getTXRate() = 0;
	virtual bool checkRXChannel(int in_chan) = 0;
	virtual bool checkTXChannel(int in_chan) = 0;
	virtual bool checkRXFreq(paramData in_param) = 0;
	virtual bool checkTXFreq(paramData in_param) = 0;
	virtual bool checkRXGain(paramData in_param) = 0;
	virtual bool checkTXGain(paramData in_param) = 0;
	virtual bool checkRXRate(paramData in_param) = 0;
	virtual bool checkTXRate(paramData in_param) = 0;
	virtual void setCustomSDRParameter(std::string name, std::string val) = 0;
	virtual std::string getCustomSDRParameter(std::string name) = 0;
	virtual void setStreamDataType(stream_type in_type) = 0;
private:
	int cur_uid = 0;
	std::map<int, int> rx_to_uid;
	std::map<int, int> tx_to_uid;
	std::map<int, int> generic_to_uid;
	std::map<std::string, paramDetails> param_accessors;
}
