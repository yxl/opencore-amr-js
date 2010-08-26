/* ------------------------------------------------------------------
 * Copyright (C) 2009 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include "wavreader.h"

#define TAG(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

uint32_t WavReader::readTag() {
	uint32_t tag = 0;
	tag = (tag << 8) | fgetc(wav);
	tag = (tag << 8) | fgetc(wav);
	tag = (tag << 8) | fgetc(wav);
	tag = (tag << 8) | fgetc(wav);
	return tag;
}

uint32_t WavReader::readInt32() {
	uint32_t value = 0;
	value |= fgetc(wav) <<  0;
	value |= fgetc(wav) <<  8;
	value |= fgetc(wav) << 16;
	value |= fgetc(wav) << 24;
	return value;
}

uint16_t WavReader::readInt16() {
	uint16_t value = 0;
	value |= fgetc(wav) << 0;
	value |= fgetc(wav) << 8;
	return value;
}

WavReader::WavReader(const char *filename) {
	dataLength = 0;
	format = 0;
	sampleRate = 0;
	bitsPerSample = 0;
	channels = 0;
	byteRate = 0;
	blockAlign = 0;

	wav = fopen(filename, "rb");
	if (wav == NULL)
		return;

	long dataPos = 0;
	while (true) {
		uint32_t tag = readTag();
		if (feof(wav))
			break;
		uint32_t length = readInt32();
		if (tag != TAG('R', 'I', 'F', 'F') || length < 4) {
			fseek(wav, length, SEEK_CUR);
			continue;
		}
		uint32_t tag2 = readTag();
		length -= 4;
		if (tag2 != TAG('W', 'A', 'V', 'E')) {
			fseek(wav, length, SEEK_CUR);
			continue;
		}
		// RIFF chunk found, iterate through it
		while (length >= 8) {
			uint32_t subtag = readTag();
			if (feof(wav))
				break;
			uint32_t sublength = readInt32();
			length -= 8;
			if (length < sublength)
				break;
			if (subtag == TAG('f', 'm', 't', ' ')) {
				if (sublength < 16) {
					// Insufficient data for 'fmt '
					break;
				}
				format = readInt16();
				channels = readInt16();
				sampleRate = readInt32();
				byteRate = readInt32();
				blockAlign = readInt16();
				bitsPerSample = readInt16();
			} else if (subtag == TAG('d', 'a', 't', 'a')) {
				dataPos = ftell(wav);
				dataLength = sublength;
				fseek(wav, sublength, SEEK_CUR);
			} else {
				fseek(wav, sublength, SEEK_CUR);
			}
			length -= sublength;
		}
		if (length > 0) {
			// Bad chunk?
			fseek(wav, length, SEEK_CUR);
		}
	}
	fseek(wav, dataPos, SEEK_SET);
}

WavReader::~WavReader() {
	fclose(wav);
}

bool WavReader::getHeader(int* format, int* channels, int* sampleRate, int* bitsPerSample, unsigned int* dataLength) {
	if (format)
		*format = this->format;
	if (channels)
		*channels = this->channels;
	if (sampleRate)
		*sampleRate = this->sampleRate;
	if (bitsPerSample)
		*bitsPerSample = this->bitsPerSample;
	if (dataLength)
		*dataLength = this->dataLength;
	return this->format && this->sampleRate;
}

int WavReader::readData(unsigned char* data, unsigned int length) {
	if (wav == NULL)
		return -1;
	if (length > dataLength)
		length = dataLength;
	int n = fread(data, 1, length, wav);
	dataLength -= length;
	return n;
}

