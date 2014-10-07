#include "tccengine.h"
#include "tcc/libtcc.h"
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
 	tcc_add_file(tce->state, "tcc/libtcc1-x86_64.a");
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


