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

