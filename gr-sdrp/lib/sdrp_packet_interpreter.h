#ifndef INCLUDED_SDRP_PACKET_INTERPRETER_H
#define INCLUDED_SDRP_PACKET_INTERPRETER_H

#include <gr_core_api.h>
#include <gr_sync_block.h>
#include <gr_message.h>
#include <gr_msg_queue.h>
#include <gr_pdu.h>

class sdrp_packet_interpreter;
typedef boost::shared_ptr<sdrp_packet_interpreter> sdrp_packet_interpreter_sptr;

GR_CORE_API sdrp_packet_interpreter_sptr sdrp_make_packet_interpreter ();

/*!
 * \brief Turn received messages into a stream
 * \ingroup source_blk
 */
class GR_CORE_API sdrp_packet_interpreter : public gr_sync_block {
private:
	gr_pdu_vector_type    d_vectortype;
	size_t 	        d_itemsize;
	std::vector<uint8_t>  prim_array;

	void sdrp_packet_interpreter::inSocketMsg(pmt::pmt_t msg);
	void sdrp_packet_interpreter::inPacketMsg(pmt::pmt_t msg);

	friend GR_CORE_API sdrp_packet_interpreter_sptr
		gr_make_pdu_to_tagged_stream();

protected:
	sdrp_packet_interpreter ();

public:
	~sdrp_packet_interpreter ();

	int work (int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);

};

#endif /* INCLUDED_SDRP_PACKET_INTERPRETER_H */
