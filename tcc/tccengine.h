#ifndef _TCCENGINE_H
#define _TCCENGINE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TCCState TCCState;
typedef struct TCCEngine TCCEngine;

typedef void (*PopulateTCCEFunction)(void * tce, int is_c );
typedef void (*TCELinkage)( void * v );

struct TCCEngine
{
	//Dupped
	const char * filename;
	double readtime;
	TCCState * state;
	PopulateTCCEFunction popfn;
	void * image;

	TCELinkage start;//called any time the script is loaded/reloaded
	TCELinkage stop;
	void * id;
};


TCCEngine * TCCECreate( const char * tccfile, PopulateTCCEFunction popfn );
int TCCECheck( TCCEngine * tce ); //returns 1 if re-compiling good. 0 if nothing was done.
void TCCEDestroy( TCCEngine * tce );
void * TCCEGetSym( TCCEngine * tce, const char * symname );
void TCCESetSym( TCCEngine * tce, const char * symname, void * sym );

#ifdef __cplusplus
};
#endif


#endif

