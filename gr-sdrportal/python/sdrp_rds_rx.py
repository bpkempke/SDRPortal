#!/usr/bin/env python

from gnuradio import gr, rds, audio, blks2
from gnuradio.eng_option import eng_option
from optparse import OptionParser
from grc_gnuradio import blks2 as grc_blks2
import sys, math, wx, time

class rds_rx_graph (gr.top_block):
	def __init__(self):
		gr.top_block.__init__ (self)

		parser=OptionParser(option_class=eng_option)
		parser.add_option("-H", "--hostname", type="string", default="localhost",
						  help="set hostname of generic sdr")
		parser.add_option("-P", "--portnum", type="int", default=None,
						  help="set portnum of generic sdr")
		parser.add_option("-r", "--sdr_rate", type="eng_float", default=250e3,
						  help="set sample rate of generic sdr")
		parser.add_option("-V", "--volume", type="eng_float", default=None,
						  help="set volume (default is midpoint)")
		parser.add_option("-O", "--audio-output", type="string", default="plughw:0,0",
						  help="pcm device name (default is plughw:0,0)")

		# print help when called with wrong arguments
		(options, args) = parser.parse_args()
		if len(args) != 0:
			parser.print_help()
			sys.exit(1)

		self.vol = options.volume
		if self.vol is None:
			self.vol = 0.1

		# connect to generic SDR
		sdr_rate = options.sdr_rate
		audio_decim = 8
		audio_rate = int(sdr_rate/audio_decim)
		print "audio_rate = ", audio_rate
		self.interleaved_short_to_complex = gr.interleaved_short_to_complex()
		self.char_to_short = gr.char_to_short(1)
		self.sdr_source = grc_blks2.tcp_source(
			itemsize=gr.sizeof_char*1,
			addr=options.hostname,
			port=options.portnum,
			server=False
		)
#		self.sdr_source = gr.file_source(1, 'sdrs_baseband.dat')
#		self.throttle = gr.throttle(1, 500e3)
#		self.connect(self.sdr_source, self.file_sink)
		self.logger = gr.file_sink(1, 'log.out')
		self.connect(self.sdr_source, self.logger)


		# channel filter
		chan_filter_coeffs = gr.firdes.low_pass(
			1.0,			# gain
			sdr_rate,		# sampling rate
			80e3,			# passband cutoff
			35e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.chan_filter = gr.fir_filter_ccf(1, chan_filter_coeffs)
		print "# channel filter:", len(chan_filter_coeffs), "taps"

		# PLL-based WFM demod
		fm_alpha = 0.25 * 250e3 * math.pi / sdr_rate		# 0.767
		fm_beta = fm_alpha * fm_alpha / 4.0			# 0.147
		fm_max_freq = 2.0 * math.pi * 90e3 / sdr_rate		# 2.209
		self.fm_demod = gr.pll_freqdet_cf(
			1.0,				# Loop BW
			fm_max_freq,		# in radians/sample
			-fm_max_freq)
		self.fm_demod.set_alpha(fm_alpha)
		self.fm_demod.set_beta(fm_beta)
		self.connect(self.sdr_source, self.char_to_short)
		self.connect(self.char_to_short, self.interleaved_short_to_complex)
		self.connect(self.interleaved_short_to_complex, self.chan_filter, self.fm_demod)

		# L+R, pilot, L-R, RDS filters
		lpr_filter_coeffs = gr.firdes.low_pass(
			1.0,			# gain
			sdr_rate,		# sampling rate
			15e3,			# passband cutoff
			1e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.lpr_filter = gr.fir_filter_fff(audio_decim, lpr_filter_coeffs)
		pilot_filter_coeffs = gr.firdes.band_pass(
			1.0,			# gain
			sdr_rate,		# sampling rate
			19e3-500,		# low cutoff
			19e3+500,		# high cutoff
			1e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.pilot_filter = gr.fir_filter_fff(1, pilot_filter_coeffs)
		dsbsc_filter_coeffs = gr.firdes.band_pass(
			1.0,			# gain
			sdr_rate,		# sampling rate
			38e3-15e3/2,	# low cutoff
			38e3+15e3/2,	# high cutoff
			1e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.dsbsc_filter = gr.fir_filter_fff(1, dsbsc_filter_coeffs)
		rds_filter_coeffs = gr.firdes.band_pass(
			1.0,			# gain
			sdr_rate,		# sampling rate
			57e3-3e3,		# low cutoff
			57e3+3e3,		# high cutoff
			3e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.rds_filter = gr.fir_filter_fff(1, rds_filter_coeffs)
		print "# lpr filter:", len(lpr_filter_coeffs), "taps"
		print "# pilot filter:", len(pilot_filter_coeffs), "taps"
		print "# dsbsc filter:", len(dsbsc_filter_coeffs), "taps"
		print "# rds filter:", len(rds_filter_coeffs), "taps"
		self.connect(self.fm_demod, self.lpr_filter)
		self.connect(self.fm_demod, self.pilot_filter)
		self.connect(self.fm_demod, self.dsbsc_filter)
		self.connect(self.fm_demod, self.rds_filter)

		# down-convert L-R, RDS
		self.stereo_baseband = gr.multiply_ff()
		self.connect(self.pilot_filter, (self.stereo_baseband, 0))
		self.connect(self.pilot_filter, (self.stereo_baseband, 1))
		self.connect(self.dsbsc_filter, (self.stereo_baseband, 2))
		self.rds_baseband = gr.multiply_ff()
		self.connect(self.pilot_filter, (self.rds_baseband, 0))
		self.connect(self.pilot_filter, (self.rds_baseband, 1))
		self.connect(self.pilot_filter, (self.rds_baseband, 2))
		self.connect(self.rds_filter, (self.rds_baseband, 3))

		# low-pass and downsample L-R
		lmr_filter_coeffs = gr.firdes.low_pass(
			1.0,			# gain
			sdr_rate,		# sampling rate
			15e3,			# passband cutoff
			1e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.lmr_filter = gr.fir_filter_fff(audio_decim, lmr_filter_coeffs)
		self.connect(self.stereo_baseband, self.lmr_filter)

		# create L, R from L-R, L+R
		self.left = gr.add_ff()
		self.right = gr.sub_ff()
		self.connect(self.lpr_filter, (self.left, 0))
		self.connect(self.lmr_filter, (self.left, 1))
		self.connect(self.lpr_filter, (self.right, 0))
		self.connect(self.lmr_filter, (self.right, 1))

		# volume control, complex2flot, audio sink
		self.volume_control_l = gr.multiply_const_ff(self.vol)
		self.volume_control_r = gr.multiply_const_ff(self.vol)
		output_audio_rate = 48000
		self.resamp_L = blks2.rational_resampler_fff(interpolation=output_audio_rate,decimation=audio_rate,taps=None,fractional_bw=None,)
		self.resamp_R = blks2.rational_resampler_fff(interpolation=output_audio_rate,decimation=audio_rate,taps=None,fractional_bw=None,)
		self.connect(self.left,  self.volume_control_l, self.resamp_L)
		self.connect(self.right, self.volume_control_r, self.resamp_R)
#		self.audio_sink = audio.sink(int(output_audio_rate),
#							options.audio_output, False)
# 		self.connect(self.resamp_L, (self.audio_sink, 0))
#		self.connect(self.resamp_R, (self.audio_sink, 1))
		self.file_sink1 = gr.file_sink(gr.sizeof_float, 'audioL.dat')
		self.file_sink2 = gr.file_sink(gr.sizeof_float, 'audioR.dat')
		self.file_sink3 = gr.file_sink(gr.sizeof_float, 'fmDemod.dat')
       		self.connect(self.resamp_L, self.file_sink1)
		self.connect(self.resamp_R, self.file_sink2)
		self.connect(self.fm_demod, self.file_sink3)

		# low-pass the baseband RDS signal at 1.5kHz
		rds_bb_filter_coeffs = gr.firdes.low_pass(
			1,				# gain
			sdr_rate,		# sampling rate
			1.5e3,			# passband cutoff
			2e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.rds_bb_filter = gr.fir_filter_fff(audio_decim, rds_bb_filter_coeffs)
		print "# rds bb filter:", len(rds_bb_filter_coeffs), "taps"
		self.connect(self.rds_baseband, self.rds_bb_filter)

		# 1187.5bps = 19kHz/16
		self.clock_divider = rds.freq_divider(16)
		rds_clock_taps = gr.firdes.low_pass(
			1,				# gain
			sdr_rate,		# sampling rate
			1.2e3,			# passband cutoff
			1.5e3,			# transition width
			gr.firdes.WIN_HAMMING)
		self.rds_clock = gr.fir_filter_fff(audio_decim, rds_clock_taps)
		print "# rds clock filter:", len(rds_clock_taps), "taps"
		self.connect(self.pilot_filter, self.clock_divider, self.rds_clock)

		# bpsk_demod, diff_decoder, rds_decoder
		self.bpsk_demod = rds.bpsk_demod(audio_rate)
		self.differential_decoder = gr.diff_decoder_bb(2)
		self.msgq = gr.msg_queue()
		self.rds_decoder = rds.data_decoder(self.msgq)
		self.connect(self.rds_bb_filter, (self.bpsk_demod, 0))
		self.connect(self.rds_clock, (self.bpsk_demod, 1))
		self.connect(self.bpsk_demod, self.differential_decoder)
		self.connect(self.differential_decoder, self.rds_decoder)


if __name__ == '__main__':
	try:
		tb = rds_rx_graph();
		tb.start();
		print "***STARTED***"
		tb.wait();
	except KeyboardInterrupt:
		pass
