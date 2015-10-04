#ifndef INCLUDED_CCSDS_TM_DERANDOMIZER_H
#define INCLUDED_CCSDS_TM_DERANDOMIZER_H

#include <sdrp/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace sdrp {

    class SDRP_API ccsds_tm_derandomizer : virtual public gr::sync_block
    {
    public:
      
      typedef boost::shared_ptr<ccsds_tm_derandomizer> sptr;

      static sptr make(const std::string &tag_name);
      virtual void setDerandomizerEn(bool rand_en) = 0;
    };

  } /* namespace sdrp */
} /* namespace gr */

#endif
