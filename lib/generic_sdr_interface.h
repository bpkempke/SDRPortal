
class genericSDRInterface {
public:
	void setSDRParameter(std::string name, std::string val);
protected:
	virtual void setRXChannel(int chan) = 0;
	virtual void setTXChannel(int chan) = 0;
	virtual void setRXFreq(double freq) = 0;
	virtual void setTXFreq(double freq) = 0;
	virtual void setRXGain(double gain) = 0;
	virtual void setTXGain(double gain) = 0;
	virtual void setRXRate(double rate) = 0;
	virtual void setTXRate(double rate) = 0;
	virtual int getRXChannel() = 0;
	virtual int getTXChannel() = 0;
	virtual double getRXFreq() = 0;
	virtual double getTXFreq() = 0;
	virtual double getRXGain() = 0;
	virtual double getTXGain() = 0;
	virtual double getRXRate() = 0;
	virtual double getTXRate() = 0;
	virtual bool checkRXChannel(int chan) = 0;
	virtual bool checkTXChannel(int chan) = 0;
	virtual bool checkRXFreq(double freq) = 0;
	virtual bool checkTXFreq(double freq) = 0;
	virtual bool checkRXGain(double gain) = 0;
	virtual bool checkTXGain(double gain) = 0;
	virtual bool checkRXRate(double rate) = 0;
	virtual bool checkTXRate(double rate) = 0;
	virtual void setCustomSDRParameter(std::string name, std::string val) = 0;
	virtual std::string getCustomSDRParameter(std::string name) = 0;
}
