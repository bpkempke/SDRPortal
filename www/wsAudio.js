
function wsAudio(host_addr, progress_window, audio_rate){
	this.host_addr = host_addr;
	this.progress_window = progress_window;
	this.audio_rate = audio_rate;
	this.playing = false;
	this.next_audio_time = 0;

	//Set up audio context
	try{
		window.AudioContext = window.AudioContext||window.webkitAudioContext;
		this.audio_context = new AudioContext();
	} catch(e){
		alert('Web Audio API is not supported in this browser');
	}

	var self = this;

	this.connect = connect;
	function connect(){
		self.setStatus("Opening wsAudio connection to " + self.host_addr);
		self.ws = new WebSocket("ws://" + self.host_addr + "/range_ws","range_ws");
		self.ws.binaryType = "arraybuffer";
		self.ws.onmessage = self.onMessage;
		self.ws.onclose = self.onClose;
		self.ws.onopen = self.onOpen;
		self.state = portalCMD.STATE.INIT;
	}

	this.setStatus = setStatus;
	function setStatus(in_status){
		self.progress_window.style.visibility = "visible";
		self.progress_window.innerHTML = in_status;
	}

	this.onMessage = onMessage;
	function onMessage(evt){
		var incoming_data = new Float32Array(evt.data);

		//Construct an audiobuffer to push to the audio interface
		var buffer = self.audio_context.createBuffer(2, incoming_data.length/2, self.audio_rate);
		var left_array = buffer.getChannelData(0);
		var right_array = buffer.getChannelData(1);
		var lr_idx = 0;
		for(var ii=0; ii < incoming_data.length; ii=ii+2){
			left_array[lr_idx] = incoming_data[ii];
			right_array[lr_idx] = incoming_data[ii+1];
			lr_idx++;
		}

		//Add some delay if this is the first time that we're playing sound
		if(self.playing == false){
			self.next_audio_time = self.audio_context.currentTime + 0.1;
			self.playing = true;
		}
		
		var source = self.audio_context.createBufferSource();
		source.buffer = buffer;
		source.connect(self.audio_context.destination);
		source.start(self.next_audio_time);

		self.next_audio_time += buffer.duration;
	}

	this.onClose = onClose;
	function onClose(){

	}

	this.onOpen = onOpen;
	function onOpen(){

	}

}
