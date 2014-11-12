.PHONY: all clean

all: amrnb.js

amrnb.js: amrnb.bc pre.js
	em++ amrnb.bc -O3 -s NO_EXIT_RUNTIME=1 -s \
		EXPORTED_FUNCTIONS="['_Decoder_Interface_init','_Decoder_Interface_exit','_Decoder_Interface_Decode','_Encoder_Interface_init','_Encoder_Interface_exit', '_Encoder_Interface_Encode']" \
		--pre-js pre.js --memory-init-file 0 -o $@

amrnb.bc: bld.sh
	./bld.sh

clean:
	rm -rf build
	rm -f amrnb.*

