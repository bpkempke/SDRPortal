
function shellCMD(host_addr, progress_window){
	this.host_addr = host_addr;
	this.progress_window = progress_window;
	var self = this;

	this.connect = connect;
	function connect(connect_callback, disconnect_callback){
		self.setStatus("Opening shell connection to " + self.host_addr);
		self.ws = new WebSocket("ws://" + self.host_addr + "/range_ws","range_ws");
		self.ws.onmessage = self.onMessage;
		self.ws.onclose = self.onClose;
		self.ws.onopen = self.onOpen;
		self.connect_callback = connect_callback;
		self.disconnect_callback = disconnect_callback;
	}

	this.disconnect = disconnect;
	function disconnect(){
		self.ws.close();
	}

	this.setStatus = setStatus;
	function setStatus(in_status){
		self.progress_window.style.visibility = "visible";
		self.progress_window.innerHTML = in_status;
	}

	this.onMessage = onMessage;
	function onMessage(evt){
		var message = evt.data;
		self.command_callback(message);
	}

	this.sendShellCommand = sendShellCommand;
	function sendShellCommand(in_command, command_callback){
		self.ws.send(in_command);
		self.command_callback = command_callback;
	}

	this.onClose = onClose;
	function onClose(){
		self.disconnect_callback();
	}

	this.onOpen = onOpen;
	function onOpen(){
		self.connect_callback();
	}

}
