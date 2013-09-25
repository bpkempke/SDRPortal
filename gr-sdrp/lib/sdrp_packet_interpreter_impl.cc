#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sdrp_packet_interpreter_impl.h"
#include <gnuradio/io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <string.h>

// public constructor that returns a shared_ptr

namespace gr {
namespace sdrp {

sdrp_packet_interpreter::sptr
sdrp_packet_interpreter::make()
{
	return gnuradio::get_initial_sptr(new sdrp_packet_interpreter_impl());
}

sdrp_packet_interpreter_impl::sdrp_packet_interpreter_impl ()
	: sync_block("sdrp_packet_interpreter_impl",
			io_signature::make(0, 0, 0),
			io_signature::make(0, 0, 0))
{
	//Message ports to/from socket interface
	message_port_register_in(pmt::mp("socket_pdu_in"));
	set_msg_handler(pmt::mp("socket_pdu_in"), boost::bind(&sdrp_packet_interpreter_impl::inSocketMsg, this, _1));
	message_port_register_out(pmt::mp("socket_pdu_out"));

	//Message ports to/from flowgraph
	message_port_register_in(pmt::mp("sdrp_pdu_in"));
	set_msg_handler(pmt::mp("sdrp_pdu_in"), boost::bind(&sdrp_packet_interpreter_impl::inPacketMsg, this, _1));
	message_port_register_out(pmt::mp("sdrp_pdu_out"));

}

sdrp_packet_interpreter_impl::~sdrp_packet_interpreter_impl(){

}

void sdrp_packet_interpreter_impl::inSocketMsg(pmt::pmt_t msg){
	//This should be purely a message containing uint8's
	pmt::pmt_t msg_vector = pmt::cdr(msg);
	size_t msg_len = pmt::length(msg_vector);
	size_t offset = 0;
	uint8_t *msg_array = (uint8_t*)(pmt::uniform_vector_elements(msg_vector, offset));

	int msg_array_idx = 0;
	bool repeat = false;

	do{
		//Populate primary length if it's not populated yet
		if(prim_array.size() < sizeof(uint32_t)){
			while(prim_array.size() < sizeof(uint32_t) && msg_array_idx < msg_len)
				prim_array.push_back(msg_array[msg_array_idx++]);
		}
	
		//If the prim_array contains the length field, we can continue to parse
		if(prim_array.size() >= sizeof(uint32_t)){
			uint32_t parsed_prim_len;
			memcpy(&parsed_prim_len, &prim_array[0], sizeof(uint32_t));
	
			//Now that we know how long the primary packet is, we can continue to fill prim_array up
			while(prim_array.size() < parsed_prim_len && msg_array_idx < msg_len)
				prim_array.push_back(msg_array[msg_array_idx++]);
	
			//If we have a full primary packet, parse it!
			if(prim_array.size() == parsed_prim_len){
				int prim_array_idx = sizeof(uint32_t);
				pmt::pmt_t new_message_dict = pmt::make_dict();
				while(prim_array_idx < prim_array.size()){
					uint32_t id, sublen;
					memcpy(&id, &prim_array[prim_array_idx], sizeof(uint32_t));
					memcpy(&sublen, &msg_array[prim_array_idx+sizeof(uint32_t)], sizeof(uint32_t));

					pmt::pmt_t key = pmt::from_long((long)(id));
					pmt::pmt_t value = pmt::init_u8vector(sublen-sizeof(uint32_t)*2, (const uint8_t*)&prim_array[prim_array_idx+sizeof(uint32_t)*2]);

					new_message_dict = pmt::dict_add(new_message_dict, key, value);

					prim_array_idx += sublen;
				}

				//Create a message and send it out now that the whole prim packet has been parsed!
				pmt::pmt_t msg = pmt::cons(new_message_dict, pmt::PMT_NIL);
				message_port_pub( pmt::mp("sdrp_pdu_out"), msg );

				repeat = true;
			} else repeat = false;
		}
	} while(repeat == true);
}

void sdrp_packet_interpreter_impl::inPacketMsg(pmt::pmt_t msg){
	std::cout << "GOT AN INCOMING PACKET MESSAGE" << std::endl;
	//This should be a message containing a variety of key-value dictionary pairs
	uint8_t *buf;
	uint32_t total_buf_length = 4;
	
	// grab the components of the pdu message
	pmt::pmt_t meta(pmt::car(msg)); // make sure this is NIL || Dict ?
	pmt::pmt_t vect(pmt::cdr(msg)); // make sure this is a vector?

	//We are only interested in the metadata
	//  There should be nothing contained in the cdr
	if(!pmt::eq(meta, pmt::PMT_NIL)) {
		pmt::pmt_t keys = pmt::dict_keys(meta);
		pmt::pmt_t vals = pmt::dict_values(meta);
		size_t nitems = pmt::length(keys);
		for(size_t i = 0; i < nitems; i++) {
			total_buf_length += 8;
			total_buf_length += pmt::length(pmt::nth(i, vals));
		}
	}
	std::cout << "total_buf_length = " << total_buf_length << std::endl;

	//Allocate the buffer now that we know how long it's going to be
	buf = new uint8_t[total_buf_length];

	//Now loop through everything again, populating the 
	memcpy(&buf[0], &total_buf_length, 4);
	uint32_t cur_buf_idx = 4;
	if(!pmt::eq(meta, pmt::PMT_NIL)) {
		pmt::pmt_t keys = pmt::dict_keys(meta);
		pmt::pmt_t vals = pmt::dict_values(meta);
		size_t nitems = pmt::length(keys);
		for(size_t i = 0; i < nitems; i++) {
			uint32_t this_id = (uint32_t)(pmt::to_long(pmt::nth(i, keys)));
			uint32_t this_length = (uint32_t)(pmt::length(pmt::nth(i, vals)));
			memcpy(&buf[cur_buf_idx], &this_id, 4);
			memcpy(&buf[cur_buf_idx+4], &this_length, 4);
			size_t offset = 0;
			memcpy(&buf[cur_buf_idx+8], pmt::uniform_vector_elements(pmt::nth(i, vals),offset), pmt::length(pmt::nth(i, vals)));
			cur_buf_idx += 8 + this_length;
		}
	}

//	for(uint32_t ii=0; ii < total_buf_length; ii++){
//		std::cout << (int)buf[ii] << std::endl;
//	}

	//Construct the resulting message
	pmt::pmt_t vector = pmt::init_u8vector(total_buf_length, (const uint8_t*)&buf[0]);
	pmt::pmt_t pdu = pmt::cons( pmt::PMT_NIL, vector);

	//Finally publish the message to the socket_msg_out port
	message_port_pub( pmt::mp("socket_pdu_out"), pdu );
	delete [] buf;
}

int
sdrp_packet_interpreter_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){   

	//There should not be a reason to be in this function, correct?
	return noutput_items;
}

} /* namespace sdrp */
} /* namespace gr */
