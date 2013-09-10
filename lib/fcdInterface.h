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

#ifndef FCD_INTERFACE_H
#define FCD_INTERFACE_H

#include <vector>
#include "genericSDRInterface.h"
#include "hierarchicalDataflowBlock.h"
#include "streamConverter.h"

class fcdInterface : public genericSDRInterface{
public:
	fcdInterface(int index);
	virtual ~fcdInterface();

	void *rxData(const char *in_buffer, unsigned long num_frames);

	//Certain functions inherited from genericSDRInterface class
	virtual void setRXFreq(paramData in_param);
	virtual void setRXGain(paramData in_param);
	virtual void setRXRate(paramData in_param);
	virtual paramData getRXFreq(rxtxChanInfo in_chan);
	virtual paramData getRXGain(rxtxChanInfo in_chan);
	virtual paramData getRXRate(rxtxChanInfo in_chan);
	virtual bool checkRXChannel(int in_chan);
	virtual void openRXChannel(int in_chan);
	virtual bool checkRXFreq(paramData in_param);
	virtual bool checkRXGain(paramData in_param);
	virtual bool checkRXRate(paramData in_param);
	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan);
	virtual void disconnect();
	virtual void connect();

private:
	PaStream *fcd_dev;
	bool is_receiving;
	bool rx_cancel;
	pthread_t rx_listener;

};

#endif
