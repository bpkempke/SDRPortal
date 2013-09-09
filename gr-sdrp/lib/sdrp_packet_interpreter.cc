#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sdrp_packet_interpreter.h>
#include <gr_io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <string.h>

// public constructor that returns a shared_ptr

sdrp_packet_interpreter_sptr
sdrp_make_packet_interpreter()
{
	return gnuradio::get_initial_sptr(new sdrp_packet_interpreter());
}

sdrp_packet_interpreter::sdrp_packet_interpreter ()
	: gr_sync_block("sdrp_packet_interpreter",
			gr_make_io_signature(0, 0, 0),
			gr_make_io_signature())
{
	//Message ports to/from socket interface
	message_port_register_in(pmt::mp("socket_pdu_in"));
	set_msg_handler(pmt::mp("socket_msg_in"), boost::bind(&sdrp_packet_interpreter::inSocketMsg, this, _1));
	message_port_register_out(pmt::mp("socket_pdu_out"));

	//Message ports to/from flowgraph
	message_port_register_in(pmt::mp("sdrp_pdu_in"));
	set_msg_handler(pmt::mp("packet_msg_in"), boost::bind(&sdrp_packet_interpreter::inPacketMsg, this, _1));
	message_port_register_out(pmt::mp("sdrp_pdu_out"));

}

sdrp_packet_interpreter::~sdrp_packet_interpreter(){

}

void sdrp_packet_interpreter::inSocketMsg(pmt::pmt_t msg){
	//This should be purely a message containing uint8's
	pmt::pmt_t msg_vector = pmt::pmt_cdr(msg);
	size_t msg_len = pmt::pmt_length(vector);
	uint8_t *msg_array = (uint8_t*)(pmt::pmt_uniform_vector_elements(msg_vector, 0));

	int msg_array_idx = 0;
	bool repeat

	do{
		//Populate primary length if it's not populated yet
		if(prim_array.length() < sizeof(uint32_t)){
			while(prim_array.length() < sizeof(uint32_t) && msg_array_idx < msg_len)
				prim_array.push_back(msg_array[msg_array_idx++]);
		}
	
		//If the prim_array contains the length field, we can continue to parse
		if(prim_array.length() >= sizeof(uint32_t)){
			uint32_t parsed_prim_len;
			memcpy(&parsed_prim_len, &prim_array[0], sizeof(uint32_t));
	
			//Now that we know how long the primary packet is, we can continue to fill prim_array up
			while(prim_array.length() < parsed_prim_len && msg_array_idx < msg_len)
				prim_array.push_back(msg_array[msg_array_idx++]);
	
			//If we have a full primary packet, parse it!
			if(prim_array.length() == parsed_prim_len){
				int prim_array_idx = sizeof(uint32_t);
				pmt::pmt_t new_message_dict = pmt::pmt_make_dict();
				while(prim_array_idx < prim_array.length()){
					uint32_t id, sublen;
					memcpy(&id, &prim_array[prim_array_idx], sizeof(uint32_t));
					memcpy(&sublen, &msg_array[prim_array_idx+sizeof(uint32_t)], );

					pmt::pmt_t key = pmt::pmt_integer((long)(id));
					pmt::pmt_t value = pmt::pmt_init_u8vector(sublen-sizeof(uint32_t)*2, (const uint8_t*)&prim_array[prim_array_idx+sizeof(uint32_t)*2]);

					new_message_dict = pmt::pmt_dict_add(new_message_dict, key, value);

					prim_array_idx += sublen;
				}

				//Create a message and send it out now that the whole prim packet has been parsed!
				pmt::pmt_t msg = pmt::pmt_cons(new_message_dict, pmt::PMT_NIL);
				message_port_pub( pmt::mp("sdrp_pdu_out"), msg );

				repeat = true;
			} else repeat = false;
		}
	} while(repeat == true);
}

void sdrp_packet_interpreter::inPacketMsg(pmt::pmt_t msg){
	//This should be a message containing a variety of key-value dictionary pairs
	uint8_t *buf;
	uint32_t total_buf_length = 4;
	
	// grab the components of the pdu message
	pmt::pmt_t meta(pmt::pmt_car(msg)); // make sure this is NIL || Dict ?
	pmt::pmt_t vect(pmt::pmt_cdr(msg)); // make sure this is a vector?

	//We are only interested in the metadata
	//  There should be nothing contained in the cdr
	if(!pmt_eq(meta, pmt::PMT_NIL)) {
		pmt_t keys = pmt_dict_keys(extras);
		pmt_t vals = pmt_dict_values(extras);
		size_t nitems = pmt_length(keys);
		for(size_t i = 0; i < nitems; i++) {
			total_buf_length += 8;
			total_buf_length += pmt::pmt_length(pmt_nth(i, vals));
		}
	}

	//Allocate the buffer now that we know how long it's going to be
	buf = new uint8_t[total_buf_length];

	//Now loop through everything again, populating the 
	memcpy(&buf[0], &total_buf_length, 4);
	uint32_t cur_buf_idx = 4;
	if(!pmt_eq(meta, pmt::PMT_NIL)) {
		pmt_t keys = pmt_dict_keys(extras);
		pmt_t vals = pmt_dict_values(extras);
		size_t nitems = pmt::pmt_length(keys);
		for(size_t i = 0; i < nitems; i++) {
			uint32_t this_id = (uint32_t)(pmt_to_long(pmt_nth(i, keys)));
			uint32_t this_length = (uint32_t)(pmt::pmt_length(pmt_nth(i, vals)));
			memcpy(&buf[cur_buf_idx], &this_id, 4);
			memcpy(&buf[cur_buf_idx+4], &this_length, 4);
			memcpy(&buf[cur_buf_idx+8], pmt_uniform_vector_elements(pmt_nth(i, vals),0), pmt::pmt_length(pmt_nth(i, vals)));
			cur_buf_idx += 8 + this_length;
		}
	}


	//Construct the resulting message
	pmt::pmt_t vector = pmt::pmt_init_u8vector(total_buf_length, (const uint8_t*)&buf[0]);
	pmt::pmt_t pdu = pmt::pmt_cons( pmt::PMT_NIL, vector);

	//Finally publish the message to the socket_msg_out port
	message_port_pub( pmt::mp("socket_pdu_out"), pdu );
	delete [] buf;
}

int
sdrp_packet_interpreter::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){   

	//There should not be a reason to be in this function, correct?
	return noutput_items;
}
