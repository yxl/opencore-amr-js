.PHONY: all clean

all: amrnb.js

amrnb.js: amrnb.bc pre.js
	em++ amrnb.bc -O3 -s NO_EXIT_RUNTIME=1 -s TOTAL_MEMORY=524288 -s TOTAL_STACK=65536 -s \
		EXPORTED_FUNCTIONS="['_Decoder_Interface_init','_Decoder_Interface_exit','_Decoder_Interface_Decode','_Encoder_Interface_init','_Encoder_Interface_exit', '_Encoder_Interface_Encode']" \
		--pre-js pre.js --memory-init-file 0 -o out.js
	echo "var AMR = (function() {" > $@
	cat out.js >> $@
	echo "return AMR; })();" >> $@

amrnb.bc: bld.sh
	./bld.sh

clean:
	rm -rf build
	rm -f amrnb.*

