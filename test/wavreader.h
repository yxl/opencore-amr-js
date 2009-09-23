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

#ifndef WAVREADER_H
#define WAVREADER_H

#include <stdio.h>
#include <stdint.h>

class WavReader {
public:
	WavReader(const char *filename);
	~WavReader();

	bool getHeader(int* format, int* channels, int* sampleRate, int* bitsPerSample, unsigned int* dataLength);
	int readData(unsigned char* data, unsigned int length);

private:
	uint32_t readTag();
	uint32_t readInt32();
	uint16_t readInt16();

	FILE *wav;
	uint32_t dataLength;

	int format;
	int sampleRate;
	int bitsPerSample;
	int channels;
	int byteRate;
	int blockAlign;
};

#endif

