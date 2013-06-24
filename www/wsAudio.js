
function wsAudio(host_addr){
	this.host_addr = host_addr;
	var self = this;

	this.connect = connect;
	function connect(){
		sdrp_ws = new WebSocket("ws://" + self.host_addr + "/range_ws","range_ws");
		sdrp_ws.binaryType = "arraybuffer";
		sdrp_ws.onmessage = self.onMessage;
		sdrp_ws.onclose = self.onClose;
		sdrp_ws.onopen = self.onOpen;
	}

	this.onMessage = onMessage;
	function onMessage(evt){

	}

	this.onClose = onClose;
	function onClose(){

	}

	this.onOpen = onOpen;
	function onOpen(){

	}

}
