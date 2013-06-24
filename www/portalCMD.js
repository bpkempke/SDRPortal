
portalCMD.STATE = {
	INIT: 0,
	OPEN: 1,
	RXRATE: 2,
	RXFREQ: 3,
	RXGAIN: 4,
	CONNECTED: 5,
	IDLE: 6,
	COMMANDING: 7
}

function portalCMD(host_addr, progress_window){
	this.host_addr = host_addr;
	this.state = portalCMD.STATE.INIT;
	this.uid = 0;
	this.cmd_queue = [];
	this.progress_window = progress_window;
	var self = this;

	this.connect = connect;
	function connect(connect_callback, disconnect_callback){
		self.setStatus("Opening SDRPortal connection to " + self.host_addr);
		self.ws = new WebSocket("ws://" + self.host_addr + "/range_ws","range_ws");
		self.ws.onmessage = self.onMessage;
		self.ws.onclose = self.onClose;
		self.ws.onopen = self.onOpen;
		self.state = portalCMD.STATE.INIT;
		self.connect_callback = connect_callback;
		self.disconnect_callback = disconnect_callback;
	}

	this.setFreq = setFreq;
	function setFreq(in_freq){
		self.sendCommand("RXFREQ", in_freq.toFixed(0));
	}

	this.setGain = setGain;
	function setGain(in_gain){
		self.sendCommand("RXGAIN", in_gain.toFixed(1));
	}

	this.setStatus = setStatus;
	function setStatus(in_status){
		self.progress_window.style.visibility = "visible";
		self.progress_window.innerHTML = in_status;
	}

	this.sendCommand = sendCommand;
	function sendCommand(in_command, in_value){
		//First make sure the command isn't already queued up.  If so, smash it
		for(var ii=0; ii < self.cmd_queue.length; ii++){
			if(cmd_queue[ii].command == in_command)
				cmd_queue.splice(ii,1);
		}
		self.cmd_queue.push({command: in_command, value: in_value});
		if(self.state != portalCMD.STATE.COMMANDING)
			self.deQCommand();
	}

	this.deQCommand = deQCommand;
	function deQCommand(){
		if(self.cmd_queue.length > 0){
			self.ws.send(self.cmd_queue[0].command + " " + self.cmd_queue[0].value + "\n");
			self.cmd_queue.shift();
			self.state = portalCMD.STATE.COMMANDING;
		} else {
			self.state = portalCMD.STATE.IDLE;
		}
	}

	this.onMessage = onMessage;
	function onMessage(evt){
		var response = evt.data;
		switch(self.state){
		case portalCMD.STATE.OPEN:
			//Format of this response: "UID: portnum"
			var parsed = response.split(':');
			self.uid = parseInt(parsed[0]);
			self.port = parseInt(parsed[1]);
			self.setStatus("Setting rate to 250000 sps");
			self.ws.send("RXRATE 250000\n");
			self.state = portalCMD.STATE.RXRATE;
			break;
		case portalCMD.STATE.RXRATE:
			//Ignore reponse for now
			self.setStatus("Setting frequency to 107100000 Hz");
			self.setFreq(107100000);
			self.state = portalCMD.STATE.RXFREQ;
			break;
		case portalCMD.STATE.RXFREQ:
			//Ignore respose for now
			self.setStatus("Setting gain to 20 dB");
			self.setGain(20);
			self.state = portalCMD.STATE.RXGAIN;
			break;
		case portalCMD.STATE.RXGAIN:
			//Ignore response for now
			self.ws.send("RXCHANNEL 0\n");
			self.state = portalCMD.STATE.CONNECTED;
		case portalCMD.STATE.CONNECTED:
			self.setStatus("SDRPortal server connected");
			self.connect_callback();
			self.state = portalCMD.STATE.IDLE;
			break;
		case portalCMD.STATE.IDLE:
			break;
		case portalCMD.STATE.COMMANDING:
			self.deQCommand();
			break;
		}
	}

	this.onClose = onClose;
	function onClose(){
		self.disconnect_callback();
	}

	this.onOpen = onOpen;
	function onOpen(){
		//First thing we want to do is send the "NEWCHANNEL" command
		self.ws.send("NEWCHANNEL\n");
		self.state = portalCMD.STATE.OPEN;
	}
}
