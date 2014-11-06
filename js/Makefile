.PHONY: all clean

all: amrnb.js

amrnb.js: amrnb.bc
	em++ $^ -O3 -s NO_EXIT_RUNTIME=1 -s \
		EXPORTED_FUNCTIONS="['_Decoder_Interface_init','_Decoder_Interface_exit','_Decoder_Interface_Decode','_Encoder_Interface_init','_Encoder_Interface_exit', '_Encoder_Interface_Encode']" \
		--pre-js amr.js -o $@

amrnb.bc: bld.sh
	./bld.sh

clean:
	rm -rf build
	rm -f amrnb.*

