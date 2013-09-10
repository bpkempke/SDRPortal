/*    SDRPortal - A generic web-based interface for SDRs
 *    Copyright (C) 2013 Ben Kempke (bpkempke@umich.edu)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UHD_INTERFACE_H
#define UHD_INTERFACE_H

#include "genericSDRInterface.h"
#include "hierarchicalDataflowBlock.h"
#include "streamConverter.h"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <map>
#include <vector>
#include <queue>

class uhdInterface : public genericSDRInterface{
public:
	//Constructor
	uhdInterface(std::string args, std::string tx_subdev, std::string rx_subdev, std::string tx_ant, std::string rx_ant, double tx_rate, double rx_rate, double tx_freq, double rx_freq, double tx_gain, double rx_gain, bool codec_highspeed);

	//Transmit data methods
	void txStart(int in_chan){};
	void txEnd(int in_chan);

	//Receive data methods
	void rxStart(int in_chan);
	int rxData(std::complex<int16_t> *rx_data_iq, int num_samples, int rx_chan);
	void *rxThread(int rx_chan);
	void *txThread(int tx_chan);

	//Accessor methods to stop tx/rx threads
	void rxStop();
	void txStop();

	//Certain functions inherited from genericSDRInterface class
	virtual void setRXFreq(paramData in_param);
	virtual void setTXFreq(paramData in_param);
	virtual void setRXGain(paramData in_param);
	virtual void setTXGain(paramData in_param);
	virtual void setRXRate(paramData in_param);
	virtual void setTXRate(paramData in_param);
	virtual paramData getRXFreq(rxtxChanInfo in_chan);
	virtual paramData getTXFreq(rxtxChanInfo in_chan);
	virtual paramData getRXGain(rxtxChanInfo in_chan);
	virtual paramData getTXGain(rxtxChanInfo in_chan);
	virtual paramData getRXRate(rxtxChanInfo in_chan);
	virtual paramData getTXRate(rxtxChanInfo in_chan);
	virtual bool checkRXChannel(int in_chan);
	virtual bool checkTXChannel(int in_chan);
	virtual void openRXChannel(int in_chan);
	virtual void openTXChannel(int in_chan);
	virtual bool checkRXFreq(paramData in_param);
	virtual bool checkTXFreq(paramData in_param);
	virtual bool checkRXGain(paramData in_param);
	virtual bool checkTXGain(paramData in_param);
	virtual bool checkRXRate(paramData in_param);
	virtual bool checkTXRate(paramData in_param);
	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan);
	virtual void disconnect();
	virtual void connect();
	virtual void txIQData(void *data, int num_bytes, int tx_chan);


private:
	void writeMAX2829Reg(uint32_t value);

	uhd::usrp::multi_usrp::sptr shared_uhd;

	//Pointers to the tx and rx streams
	std::vector<uhd::tx_streamer::sptr> tx_streams;
	std::vector<uhd::rx_streamer::sptr> rx_streams;
	std::vector<uhd::tx_metadata_t> tx_md;
	std::vector<uhd::rx_metadata_t> rx_md;

	//Queue of samples to transmit
	std::vector<std::queue<std::complex<float> > > tx_queue;

	//PThreads for rx and tx
	pthread_t rx_listener, tx_thread;

	//Special parameters just for the USRP series
	//TODO: This...

	std::string d_args, d_tx_subdev, d_rx_subdev, d_tx_ant, d_rx_ant;
	double d_tx_rate, d_rx_rate, d_tx_freq, d_rx_freq, d_tx_gain, d_rx_gain;
	bool d_codec_highspeed;
	bool rx_running, tx_running;

};

#endif

