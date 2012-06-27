#include "uhd_interface.h"
#include <cstdio>
using namespace std;

//Proxies in order to run a thread within a member function
static void *uhdReadProxy(void *in_uhd_int){
	static_cast<uhdInterface*>(in_uhd_int)->rxThread();
}
static void *uhdWriteProxy(void *in_uhd_int){
	static_cast<uhdInterface*>(in_uhd_int)->txThread();
}

/**********************
  * uhdInterface class
  *********************/
uhdInterface::uhdInterface(string args, string tx_subdev, string rx_subdev, string tx_ant, string rx_ant, double tx_rate, double rx_rate, double tx_freq, double rx_freq, double tx_gain, double rx_gain){
	//Open blank UHD object
	shared_uhd = uhd::usrp::multi_usrp::make(args);

	//Default to the internal oscillator
	shared_uhd->set_clock_source("internal");

	//If subdevice specification given, use it
	if(tx_subdev != "") shared_uhd->set_tx_subdev_spec(tx_subdev);
	if(rx_subdev != "") shared_uhd->set_rx_subdev_spec(rx_subdev);

	//Set the sample rate
	shared_uhd->set_tx_rate(tx_rate);
	shared_uhd->set_rx_rate(rx_rate);

	//Set the desired carrier frequencies
	shared_uhd->set_tx_freq(tx_freq);
	shared_uhd->set_rx_freq(rx_freq);

	//Set the desired gains
	shared_uhd->set_tx_gain(tx_gain);
	shared_uhd->set_rx_gain(rx_gain);

	//Set up the antennas
	if(tx_ant != "") shared_uhd->set_tx_antenna(tx_ant);
	if(rx_ant != "") shared_uhd->set_rx_antenna(rx_ant);

	//Set up the transmit and receive streamers (let's keep it to a float stream for now)
	uhd::stream_args_t tx_stream_args("fc32");
	tx_stream = shared_uhd->get_tx_stream(tx_stream_args);
	uhd::stream_args_t rx_stream_args("fc32","sc16");
	rx_stream = shared_uhd->get_rx_stream(rx_stream_args);

	//Other miscellaneous initialization stuff
	tx_md.start_of_burst = false;
	tx_md.end_of_burst = false;

	//We likely want to be always receiving, so let's start that up
	rxStart();
	pthread_create(&rx_listener, NULL, uhdReadProxy, (void*)this);

	//A tx thread is also instantiated which handles sending stuff from tx_queue out to the radio
	pthread_create(&tx_thread, NULL, uhdWriteProxy, (void*)this);
}

void uhdInterface::parseControlStream(string in_data){
	
}

void uhdInterface::registerDownstreamControlInterface(fdInterface *in_int, controlIntType in_int_type){
	//Make a new uhdControlConnection instance and link to it
	uhdControlConnection *new_connection = new uhdControlConnection(this, in_int, in_int_type);
	control_connections[in_int] = new_connection;

	//Connect the new uhdControlConnection with the downstream object as well
	in_int->registerUpstreamInterface(new_connection);
}

void uhdInterface::fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id){
	//Figure out where it needs to go and call that downstream fdBytesReceived method
	if(control_connections.find(from_interface) != control_connections.end()){
		control_connections[from_interface]->fdBytesReceived(buffer, num_bytes, from_interface, secondary_id);
	}
}

void uhdInterface::txData(complex<float> *tx_data_iq, int num_samples){
	//Simple function just passes all iq data out to the device
	tx_md.end_of_burst = false;
	tx_stream->send(tx_data_iq, num_samples, tx_md);
}

//This is a dummy function whose only job is to notify the uhddriver that we're done with the current tx burst
void uhdInterface::txEnd(){
	//Send a sample of dummy data out to the device with the end_of_burst bit set
	complex<float> dummy_sample;
	tx_md.end_of_burst = true;
	tx_stream->send(&dummy_sample, 1, tx_md);
	tx_md.end_of_burst = false;
}

void uhdInterface::rxStart(){
	uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
	stream_cmd.num_samps = 0;
	stream_cmd.stream_now = true;
	stream_cmd.time_spec = uhd::time_spec_t();
	shared_uhd->issue_stream_cmd(stream_cmd);
}

int uhdInterface::rxData(complex<float> *rx_data_iq, int num_samples){
	//Simple call to stream->recv
	int ret = rx_stream->recv(rx_data_iq, num_samples, rx_md, 1.0); //1.0 is the timeout time (in seconds)

	//Error checking
	if(rx_md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT)
		cout << "rx error 1";
	else if(rx_md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW)
		cout << "rx error 2";
	else if(rx_md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE){
		cout << "rx error 3";
		ret = -1;
	}

	return ret;
}

void uhdInterface::queueTXSamples(complex<float> *in_samples, int num_samples){
	tx_queue.insert(tx_queue.end(),in_samples,in_samples+num_samples);
}

//Let's break this in to chunks of 256 samples for now.  Can go up or down, but let's try this for now
#define RX_CHUNK_SIZE 256
void uhdInterface::rxThread(){
	complex<float> rx_data[RX_CHUNK_SIZE];
	while(1){
		rxData(rx_data, RX_CHUNK_SIZE);
	
		//Now we have to push it down to all downstream interfaces
		map<fdInterface*,uhdControlConnection*>::iterator conn_it;
		for(conn_it = control_connections.begin(); conn_it != control_connections.end(); conn_it++){
			if((*conn_it).second->getIntType() == CONTROL_DATA){
				(*conn_it).second->dataFromUpstream((char*)rx_data, RX_CHUNK_SIZE*sizeof(complex<float>), this);
			}
		}
	}
}

//This thread waits until a theshold size is met in order to tx a constant stream of samples out to the USRP
#define TX_THRESHOLD 2000
void uhdInterface::txThread(){
	while(1){
		usleep(5000);
		if(tx_queue.size() > TX_THRESHOLD){
			txStart();
			while(tx_queue.size() > TX_THRESHOLD){
				//TODO: Lock here
				vector<std::complex<float> > temp_tx_queue = tx_queue;
				tx_queue.clear();
				//TODO: Unlock here

				txData(&temp_tx_queue[0], temp_tx_queue.size());
			}
			txEnd();
		}
	}
}

void uhdInterface::rxEnd(){
	//TODO: Don't think there's anything we need to do here
}

/***************************
  * uhdControlConnection class
  **************************/
uhdControlConnection::uhdControlConnection(uhdInterface *in_parent_int, fdInterface *in_int, controlIntType in_int_type){
	//Just save the pointer for later use
	downstream_int = in_int;
	parent_int = in_parent_int;

	//Remember what type of interpreter this connection should be using
	int_type = in_int_type;
	cur_secondary_id = 0;
}

controlIntType uhdControlConnection::getIntType(){
	return int_type;
}

void uhdControlConnection::fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id){
	//Check to see if we have an interpreter for this id yet
	//TODO: Is this the correct syntax?!
	if(interpreters[secondary_id]){
	}

	//Now send off the data that we were originally given
	interpreters[secondary_id]->parse(buffer,num_bytes,from_interface, secondary_id);
}

void uhdControlConnection::dataFromUpstream(char *data, int num_bytes, fdInterface *from_interface){
	//This is essentially a broadcast, so we need to send it to _all_ downstream connections
	map<int,connectionInterpreter*>::iterator conn_it;
	for(conn_it = interpreters.begin(); conn_it != interpreters.end(); conn_it++){
		(*conn_it).second->getUpstreamInt()->dataFromUpstream(data, num_bytes, from_interface);
	}
}

void uhdControlConnection::registerDownstreamInterface(fdInterface *in_thread){
	if(int_type == CONTROL_DATA)
		interpreters[in_thread->secondary_id] = new dataConnectionInterpreter(parent_int, in_thread);
	else if(int_type == CONTROL_CMD)
		interpreters[in_thread->secondary_id] = new cmdConnectionInterpreter(parent_int, in_thread);
	//TODO: Memory leak since we never really get rid of dead connections?
}

void dataConnectionInterpreter::parse(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id){
	//Keep a static vector around which queues the last characters to come in so we don't have to worry about missing bytes that aren't multiples of sizeof(complex<float>)
	static vector<char> in_data_queue;
	in_data_queue.insert(in_data_queue.end(),buffer,buffer+num_bytes);

	//Transfer over however many full complex<float>'s there are now
	int num_full_samples = in_data_queue.size()/sizeof(std::complex<float>);
	uhd_int->queueTXSamples((std::complex<float>*)(&in_data_queue[0]),num_full_samples);

	//Then pop the stuff out of in_data_queue now that it's somewhere else
	in_data_queue.erase(in_data_queue.begin(),in_data_queue.begin()+num_full_samples);
}

//This function parses all incoming text-based messaging
//All commands are to be ended with a newline in order to be registered correctly
//Examples of valid messages are as follows:
//  RXFREQ 437000000
//  TXFREQ 437000000
//  RXGAIN 30
//  TXGAIN 30
//  RXRATE 1000000
//  TXRATE 1000000
//  RXANT 0
//  TXANT 0
//All of these can also be queried by following with a question mark
//  RXFREQ?
//  TXFREQ?
//  etc...
void cmdConnectionInterpreter::parse(char *in_data, int num_bytes, fdInterface *from_interface, int secondary_id){
	//Insert historic messages into a string stream so as to easily extract lines
	static stringstream command_stream;
	string in_data_string(in_data,num_bytes);
	command_stream << in_data_string;

	cout << "in cmdConnectionInterpreter::parse" << endl;

	//Now parse out incoming commands
	string current_command;
	if(!getline(command_stream, current_command).fail()){
		stringstream arg_stream(current_command);
		string command, arg1, arg2;

		//Process the command, first argument, and second argument
		arg_stream >> command;
		arg_stream >> arg1;
		arg_stream >> arg2;

		cout << command << " " << arg1 << " " << arg2 << endl;

		//Now do whatever we need to do based on the received command
		//TODO: Put in some error checking here
		if(command == "RXFREQ")
			uhd_int->getUHDObject()->set_rx_freq(atof(arg1.c_str()));
		else if(command == "TXFREQ")
			uhd_int->getUHDObject()->set_tx_freq(atof(arg1.c_str()));
		else if(command == "RXGAIN")
			uhd_int->getUHDObject()->set_rx_gain(atof(arg1.c_str()));
		else if(command == "TXGAIN")
			uhd_int->getUHDObject()->set_tx_gain(atof(arg1.c_str()));
		else if(command == "RXRATE")
			uhd_int->getUHDObject()->set_rx_rate(atof(arg1.c_str()));
		else if(command == "TXRATE")
			uhd_int->getUHDObject()->set_tx_rate(atof(arg1.c_str()));
		else if(command == "RXANT")
			uhd_int->getUHDObject()->set_rx_antenna(arg1);
		else if(command == "TXANT")
			uhd_int->getUHDObject()->set_tx_antenna(arg1);

		//Query-based commands
		char response[20];
		int response_length;
		if(command.substr(0,6) == "RXFREQ")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_rx_freq()+0.5));
		else if(command.substr(0,6) == "TXFREQ")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_tx_freq()+0.5));
		else if(command.substr(0,6) == "RXGAIN")
			response_length = sprintf(response,"%f\r\n",uhd_int->getUHDObject()->get_rx_gain());
		else if(command.substr(0,6) == "TXGAIN")
			response_length = sprintf(response,"%f\r\n",uhd_int->getUHDObject()->get_tx_gain());
		else if(command.substr(0,6) == "RXRATE")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_rx_rate()+0.5));
		else if(command.substr(0,6) == "TXRATE")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_tx_rate()+0.5));
		else if(command.substr(0,5) == "RXANT")
			response_length = sprintf(response,"%s\r\n",uhd_int->getUHDObject()->get_rx_antenna().c_str());
		else if(command.substr(0,5) == "TXANT")
			response_length = sprintf(response,"%s\r\n",uhd_int->getUHDObject()->get_tx_antenna().c_str());

		//Send off the response
		upstream_int->dataFromUpstream(response,response_length,uhd_int);
	}
}