#ifndef INCLUDED_SDRP_PACKET_INTERPRETER_IMPL_H
#define INCLUDED_SDRP_PACKET_INTERPRETER_IMPL_H

#include <sdrp/sdrp_packet_interpreter.h>

namespace gr {
namespace sdrp {

/*!
 * \brief Turn received messages into a stream
 * \ingroup source_blk
 */
class sdrp_packet_interpreter_impl : public sdrp_packet_interpreter {
private:
	size_t 	        d_itemsize;
	std::vector<uint8_t>  prim_array;

	void inSocketMsg(pmt::pmt_t msg);
	void inPacketMsg(pmt::pmt_t msg);

public:
	sdrp_packet_interpreter_impl ();
	~sdrp_packet_interpreter_impl ();

	int work (int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);

};

}
}

#endif /* INCLUDED_SDRP_PACKET_INTERPRETER_IMPL_H */
