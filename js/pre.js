var AMR = {

  /**
   * Decode AMR file to the wav file with sample rate 8000, 16 bites per sample
   * and 1 channel.
   * @param {Uint8Array} amr The raw amr file data.
   * @returns {Uint8Array} wav data if succeeded. Otherwise null.
   */
  toWAV: function(amr) {
    var decoded= this._decode(amr);
    if (!decoded) {
      return null;
    }

    var raw = new Uint8Array(decoded.buffer, decoded.byteOffset, decoded.byteLength);

    var out = new Uint8Array(raw.length + this.WAV_HEADER_SIZE);

    var offset = 0;

    var write_int16 = function(value) {
      var a = new Uint8Array(2);
      new Int16Array(a.buffer)[0] = value;
      out.set(a, offset);
      offset += 2;
    };
    var write_int32 = function(value) {
      var a = new Uint8Array(4);
      new Int32Array(a.buffer)[0] = value;
      out.set(a, offset);
      offset += 4;
    };
    var write_string = function(value) {
      var d = new TextEncoder('utf-8').encode(value);
      out.set(d, offset);
      offset += d.length;
    };

    // Write WAV HEADER

    write_string('RIFF');
    write_int32(4 + 8 + 16 + 8 + raw.length);
    write_string('WAVEfmt ');
    write_int32(16);

    var bits_per_sample = 16;
    var sample_rate = 8000;
    var channels = 1;
    var bytes_per_frame = bits_per_sample / 8 * channels;
    var bytes_per_sec = bytes_per_frame * sample_rate;
    write_int16(1);                   // Format
    write_int16(1);                   // Channels
    write_int32(sample_rate);         // Samplerate
    write_int32(bytes_per_sec);       // Bytes per sec
    write_int16(bytes_per_frame);     // Bytes per frame
    write_int16(bits_per_sample);     // Bits per sample

    write_string('data');
    write_int32(raw.length);

    out.set(raw, offset);
    return out;
  },

  /**
   * Decode AMR file to the PCM data with sample rate 8000.
   * @param {Uint8Array} amr The raw amr file data.
   * @returns {Float32Array} PCM data if succeeded. Otherwise null.
   */
  decode: function(amr) {
    var raw= this._decode(amr);
    if (!raw) {
      return null;
    }
    var out = new Float32Array(raw.length);
    for (var i = 0; i < out.length; i++) {
      out[i] = raw[i] / 0x8000;
    }
    return out;
  },

  /**
   * Decode AMR file to raw PCM data with sample rate 8000.
   * @param {Uint8Array} amr The raw amr file data.
   * @returns {Int16Array} PCM data if succeeded. Otherwise null.
   */
  _decode: function(amr) {
    // Check file header.
    if (String.fromCharCode.apply(null, amr.subarray(0, this.AMR_HEADER.length)) !== this.AMR_HEADER) {
      return null;
    }

    var decoder = this.Decoder_Interface_init();
    if (!decoder) {
      return null;
    }

    var out = new Int16Array(Math.floor(amr.length / 6 * this.PCM_BUFFER_COUNT));

    var buf = Module._malloc(this.AMR_BUFFER_COUNT);
    var decodeInBuffer = new Uint8Array(Module.HEAPU8.buffer, buf, this.AMR_BUFFER_COUNT);

    buf = Module._malloc(this.PCM_BUFFER_COUNT * 2);
    var decodeOutBuffer = new Int16Array(Module.HEAPU8.buffer, buf, this.PCM_BUFFER_COUNT);

    var inOffset = 6;
    var outOffset = 0;
    while (inOffset + 1 < amr.length &&
           outOffset + 1 < out.length) {
      // Find the package size
      var size = this.SIZES[(amr[inOffset] >> 3) & 0x0f];
      if (inOffset + size + 1 > amr.length) {
        break;
      }
      decodeInBuffer.set(amr.subarray(inOffset, inOffset + size + 1));
      this.Decoder_Interface_Decode(decoder, decodeInBuffer.byteOffset,
        decodeOutBuffer.byteOffset, 0);

      if (outOffset + this.PCM_BUFFER_COUNT > out.length) {
        var newOut = new Int16Array(out.length * 2);
        newOut.set(out.subarray(0, outOffset));
        out = newOut;
      }
      out.set(decodeOutBuffer, outOffset);
      outOffset += this.PCM_BUFFER_COUNT;
      inOffset += size + 1;
    }

    Module._free(decodeInBuffer.byteOffset);
    Module._free(decodeOutBuffer.byteOffset);

    this.Decoder_Interface_exit(decoder);
    return out.subarray(0, outOffset);
  },

  /**
   * Encode PCM dato to AMR file.
   * @param {Float32Array} pcm The PCM data to encode
   * @param {number} pcmSampleRate The sample rate of the PCM data. It should
   * not be less than 8000.
   * @param {number} [mode] The encoding mode defined in AMR.Mode. Defaults to
   * MR795.
   * @returns {Uint8Array} The raw AMR file data if succeeded. Otherwise null.
   */
  encode: function(pcm, pcmSampleRate, mode) {
    if (pcmSampleRate < 8000) {
      console.error('pcmSampleRate should not be less than 8000.');
      return null;
    }
    if (typeof mode === 'undefined') {
      mode = this.Mode.MR795;
    }

    var encoder = this.Encoder_Interface_init();
    if (!encoder) {
      return null;
    }

    var buf = Module._malloc(this.PCM_BUFFER_COUNT * 2);
    var encodeInBuffer = new Int16Array(Module.HEAPU8.buffer, buf, this.PCM_BUFFER_COUNT);

    buf = Module._malloc(this.AMR_BUFFER_COUNT);
    var encodeOutBuffer = new Uint8Array(Module.HEAPU8.buffer, buf, this.AMR_BUFFER_COUNT);

    // Convert the Float32Array of pcm to Int16Array with sample rate 8000.
    var ratio = pcmSampleRate / 8000;
    var inLength = Math.floor(pcm.length / ratio);
    var inData = new Int16Array(inLength);
    for (var i = 0; i < inLength; i++) {
      inData[i] = pcm[Math.floor(i * ratio)] * (0x8000 - 1);
    }

    var blockSize = this.SIZES[mode] + 1;
    var out = new Uint8Array(Math.ceil(inLength / this.PCM_BUFFER_COUNT * blockSize) + this.AMR_HEADER.length);

    // Write the amr header.
    out.set(new TextEncoder('utf-8').encode(this.AMR_HEADER));

    var inOffset = 0;
    var outOffset = this.AMR_HEADER.length;
    while (inOffset + this.PCM_BUFFER_COUNT < inData.length &&
           outOffset + blockSize< out.length) {
      encodeInBuffer.set(inData.subarray(inOffset, inOffset + this.PCM_BUFFER_COUNT));
      var n = this.Encoder_Interface_Encode(encoder, mode,
                                            encodeInBuffer.byteOffset,
                                            encodeOutBuffer.byteOffset, 0);
      if (n != blockSize) {
        console.error([n, blockSize]);
        break;
      }

      out.set(encodeOutBuffer.subarray(0, n), outOffset);
      inOffset += this.PCM_BUFFER_COUNT;
      outOffset += n;
    }

    Module._free(encodeInBuffer.byteOffset);
    Module._free(encodeOutBuffer.byteOffset);

    this.Encoder_Interface_exit(encoder);
    return out.subarray(0, outOffset);
  },

  /**
   * void* Decoder_Interface_init(void)
   * @returns {number} The decoder interface handler.
   */
  Decoder_Interface_init: function() {
    console.warn('Decoder_Interface_init not initialized.');
    return 0;
  },

  /**
   * void Decoder_Interface_exit(void* state)
   * @param {number} decoder interface handler.
   * @returns {void}
   */
  Decoder_Interface_exit: function(state) {
    console.warn('Decoder_Interface_exit not initialized.');
  },

  /**
   * void Decoder_Interface_Decode(void* state, const unsigned char* in, short* out, int bfi);
   * @param {number} state The decoder interface handler.
   * @param {Uint8Array} inBuffer
   * @param {Int16Array} outBuffer
   * @param {number} bfi The bad frame indicator. 0 - no bad frame; 1 - the incoming frame is damaged or suspicious.
   * @returns {void}
   */
  Decoder_Interface_Decode: function(state, inBuffer, outBuffer, bfi) {
    console.warn('Decoder_Interface_Decode not initialized.');
  },


  /**
   * void* Encoder_Interface_init(int dtx);
   * @param {number} dtx
   * @returns {number} The encoder interface handler.
   */
  Encoder_Interface_init: function(dtx) {
    console.warn('Encoder_Interface_init not initialized.');
    return 0;
  },

  /**
   * void Encoder_Interface_exit(void* state);
   * @param {number} state The encoder interface handler.
   * @returns {void}
   */
  Encoder_Interface_exit: function(state) {
    console.warn('Encoder_Interface_exit not initialized.');
  },

  /**
   * int Encoder_Interface_Encode(void* state, enum Mode mode, const short* speech, unsigned char* out, int forceSpeech);
   * @param {number} state The encoder interface handler.
   * @param {number} mode The encoding mode defined in AMR.Mode.
   * @param {Int16Array} speech The input buffer.
   * @param {Uint8Array} out The output buffer.
   * @param {number} forceSpeech Not used.
   * @returns {number}
   */
  Encoder_Interface_Encode: function(state, mode, speech, out, forceSpeech) {
    console.warn('Encoder_Interface_Encode not initialized.');
  },

  Mode: {
    MR475: 0,    /* 4.75 kbps */
    MR515: 1,    /* 5.15 kbps */
    MR59: 2,     /* 5.90 kbps */
    MR67: 3,     /* 6.70 kbps */
    MR74: 4,     /* 7.40 kbps */
    MR795: 5,    /* 7.95 kbps */
    MR102: 6,    /* 10.2 kbps */
    MR122: 7,    /* 12.2 kbps */
    MRDTX: 8     /* DTX       */
  },

  // Decoding modes and its frame sizes (bytes), respectively
  SIZES: [12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0],

  AMR_BUFFER_COUNT: 32,

  PCM_BUFFER_COUNT: 160,

  AMR_HEADER: '#!AMR\n',

  WAV_HEADER_SIZE: 44,
};

var Module = {
  canvas: {},
  print: function(text) {
    console.log(text);
  },

  _main: function() {
    AMR.Decoder_Interface_init = Module._Decoder_Interface_init;
    AMR.Decoder_Interface_exit = Module._Decoder_Interface_exit;
    AMR.Decoder_Interface_Decode = Module._Decoder_Interface_Decode;

    AMR.Encoder_Interface_init = Module._Encoder_Interface_init;
    AMR.Encoder_Interface_exit = Module._Encoder_Interface_exit;
    AMR.Encoder_Interface_Encode = Module._Encoder_Interface_Encode;
    return 0;
  }
};
