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

#include "tccengine.h"
#include <libtcc.h>
#include "os_generic.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

TCCEngine * TCCECreate( const char * tccfile, PopulateTCCEFunction  popfn )
{
	TCCEngine * ret = malloc( sizeof( TCCEngine ) );
	memset( ret, 0, sizeof( TCCEngine ) );
	ret->filename = strdup( tccfile );
	ret->popfn = popfn;
	TCCECheck( ret );
	return ret;
}

int TCCECheck( TCCEngine * tce )
{
	int r;
/*
	char * program;
	int ifl;
	FILE * f = fopen( tce->filename, "rb" );
	if( !f || ferror( f ) )
	{
		if( f ) fclose( f );
		fprintf( stderr, "Error: Cannot open TCC Tool on file: \"%s\"\n", tce->filename );
		return;
	}
	fseek( f, 0, SEEK_END );
	ifl = ftell( f );
	fseek( f, 0, SEEK_SET );
	program = malloc( ifl + 1 );
	int r = fread( program, ifl, 1, f );
	program[ifl] = 0;
	fclose( f );
	if( r < 0 )
	{
		fprintf( stderr, "Error: File I/O Error on file: \"%s\"\n", tce->filename );
		free( program );
		return;
	}*/
	double readtime = OGGetFileTime( tce->filename );
	if( tce->readtime == readtime )
	{
		return 0;
	}

	tce->readtime = readtime;
	TCCState * backuptcc = tce->state;
	
	tce->state = tcc_new();
	tcc_set_output_type(tce->state, TCC_OUTPUT_MEMORY);
	tce->popfn( tce ); //Populate everything.
	tcc_add_library( tce->state, "m" );
	tcc_add_include_path( tce->state, ".." );
	tcc_add_include_path( tce->state, "." );
	r = tcc_add_file( tce->state, tce->filename );
	if( r )
	{
		fprintf( stderr, "TCC Comple Status: %d\n", r );
		goto end_err;
	}


#if __WORDSIZE == 64
// 	tcc_add_file(tce->state, "tcc/libtcc1-x86_64.a");
#else
//#error No TCC Defined for this architecture.
#endif
	r = tcc_relocate(tce->state, NULL);
	if (r == -1)
	{
		fprintf( stderr, "TCC Reallocation error." );
		goto end_err;
	}

	//Issue stop
	if( tce->stop )
	{
		printf( "Stopping...\n" );
		tce->stop( tce->id );
		printf( "Stop done.\n" );
	}
	if( backuptcc )
		tcc_delete( backuptcc );
	if( tce->image ) free( tce->image );
	tce->image = malloc(r);
	tcc_relocate( tce->state, tce->image );

	tce->stop = (TCELinkage)tcc_get_symbol(tce->state, "stop" );
	tce->start = (TCELinkage)tcc_get_symbol(tce->state, "start" );
	printf(" Start: %p\n", tce->start );
	tce->start( tce->id );
	tce->collision = (TCELinkage)tcc_get_symbol(tce->state, "collision" );

	return 1;
end_err:
	tcc_delete( tce->state );
	tce->state = backuptcc;
	return -1;
}

void TCCEDestroy( TCCEngine * tce )
{
	tcc_delete( tce->state );
	free( tce->image );
	free( tce );
}

void * TCCEGetSym( TCCEngine * tce, const char * symname )
{
	return tcc_get_symbol(tce->state, symname );
}

void TCCESetSym( TCCEngine * tce, const char * symname, void * sym )
{
	tcc_add_symbol( tce->state, symname, sym );
}


