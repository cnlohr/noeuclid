/*
	Copyright (c) 2014 <>< Charles Lohr
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of the <organization> nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Common.h"
#include <zlib.h>


string ZLibUncompress( const string & in )
{
	string sret;

	#define ZCHUNK 256
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	int ret = inflateInit(&strm);
	if (ret != Z_OK)
	{
		fprintf( stderr, "MAJOR PROBLEM: ZLib failed to initiate\n" );
		return "";
	}
	strm.avail_in = in.length();
	strm.next_in = (Bytef*)in.c_str();
	do
	{
		char out[ZCHUNK];
		strm.avail_out = ZCHUNK;
		strm.next_out = (Bytef*)out;

		ret = inflate(&strm, Z_NO_FLUSH);
		if(ret == Z_STREAM_ERROR)
		{
			fprintf( stderr, "MAJOR PROBLEM! Z_STREAM_ERROR\n" );
			return "";
		}

		switch (ret) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;     /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			fprintf( stderr, "MAJOR PROBLEM! Z DICT, DATA, OR MEM ERROR\n" );
			(void)inflateEnd(&strm);
			return "";
		}

		int have = ZCHUNK - strm.avail_out;
		sret.append( out, have );
	} while (ret != Z_STREAM_END);

	(void)inflateEnd(&strm);
	return sret;
}

