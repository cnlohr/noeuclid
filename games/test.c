#include "scripthelpers.h"


//Initial start toom is 0	
#define START_ROOM 4

int firstrun = 0;
int lastroom = -1;
int room = START_ROOM;

void StartAtRoom( int rid )
{
	firstrun = 1;
	room = rid;
	switch( rid )
	{
	case 0:
		gPositionX = 3;
		gPositionY = 3;
		gPositionZ = 64;
		break;
	case 1:
		gPositionX = 4.28;
		gPositionY = 4.27;
		gPositionZ = 59.13;
		break;
	case 2:
		gPositionX = 4.5;
		gPositionY = 7.92;
		gPositionZ = 51.92;
		break;
	case 3:
		gPositionX = 4.5;
		gPositionY = 15.29;
		gPositionZ = 48.44;
		break;
	case 4:
		gPositionX = 6.5;
		gPositionY = 31.5;
		gPositionZ = 49;
		break;
	}
}


//Only called once.
void initialize( )
{
	printf( "Initialize.\n" );

	StartAtRoom( START_ROOM );
}

void start( void * id )
{	
	printf( "Start %d\n", gOverallUpdateNo );
	firstrun = 1;
}

void stop( void * id )
{
	printf( "Stop\n" );
}


void RunRoom0()
{
	if( firstrun )
	{
		GameTimer = 100;
		firstrun = 0;
		MakeEmptyBox    ( 1, 1, 60, 6, 6, 6, WALL_BLOCK, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 );  //White box,
		SetWarpSpaceArea( 1, 1, 60, 6, 6, 6, .1, .1, .1 );    //very big box.
		ChangeCell( 0, 4, 4, 60, 1, DEFAULT_BRIGHT, 255, GOAL_BLOCK );
	}

	if( IsPlayerInRange( 4, 4, 61, 1, 1, 1 ) )
	{
		room = 1;
	}
}

void RunRoom1()
{
	static double TimeInRoom1;
	TimeInRoom1 += worldDeltaTime;

	if( firstrun )
	{
		GameTimer = 100;
		firstrun = 0;
		TimeInRoom1 = 0.0;

		//Box off to the side
		MakeEmptyBox( 2, 6, 50, 4, 4, 3, 7, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Chute


		MakeEmptyBox( 2, 2, 48, 4, 4, 11, 4, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Chute
		ClearCell( 4, 4, 60 );
		ClearCell( 4, 4, 59 );
		int i;
		for( i = 0; i < 10; i++ )
		{
			float den = (i+1.0)/12.0;
			if( den < .1 ) den = .1;
			SetWarpSpaceArea( 2, 2, 59-i, 4, 4, 1, den, den, den );    //very big box
		}
		//Make hole in side.
		ClearRange( 3, 6, 51, 3, 1, 2 );

		//ChangeCell( 0, 4, 10, 52, 1, DEFAULT_BRIGHT, 255, GOAL_BLOCK );
		PaintRange( 4, 10, 50, 1, 1, 3, GOAL_BLOCK, 255 );
	}


	int capden = 255 - TimeInRoom1 * 200;
	float warp = ((TimeInRoom1*.1)>1)?1:((TimeInRoom1*.1)+.1);

	if( capden < 0 )
	{
		ChangeCell( 0, 4, 4, 60, 0, DEFAULT_BRIGHT, 0, GOAL_BLOCK );
	}
	else
	{
		ChangeCell( 0, 4, 4, 60, 1, DEFAULT_BRIGHT, capden, GOAL_BLOCK );
	}

	if( IsPlayerInRange( 3, 9, 51, 3, 1, 2 ) )
	{
		room = 2;
	}
}


void RunRoom2()
{
	static int StageInRoom2;
	static double TimeInRoom2;
	TimeInRoom2 += worldDeltaTime;

	gDaytime = 400;
	if( firstrun )
	{
		GameTimer = 100;
		StageInRoom2 = 0;
		TimeInRoom2 = 0;
		firstrun = 0;

		MakeEmptyBox( 2, 11, 47, 4, 4, 10, SPAAACE_CELL, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Open area
		SetWarpSpaceArea( 2, 11, 46, 4, 4, 3, .3, .3, .3 );    //very big box

		//Clear the door in.
		PaintRange( 3, 11, 47, 3, 1, 8, GOAL_BLOCK, 255 );
		ClearRange( 4, 11, 51, 1, 1, 2 );
	}


	int capden = 255 - TimeInRoom2 * 200;
	float warp = ((TimeInRoom2*.1)>1)?1:((TimeInRoom2*.1)+.1);

	if( capden < 0 )
	{
		PaintRange( 4, 10, 51, 1, 1, 2, GOAL_BLOCK, 0 );
	}
	else
	{
		PaintRange( 4, 10, 51, 1, 1, 2, GOAL_BLOCK, capden );
	}

	//Force user to look around to get out.
	switch( StageInRoom2 )
	{
	case 0:
		if( gDirectionX > .7 && IsPlayerInRange( 3, 12, 48, 3, 3, 3 ) ) StageInRoom2 = 1;
		break;
	case 1:
		if( gDirectionX < -.7 ) StageInRoom2 = 2;
		break;
	case 2:
		if( gDirectionX > .7 ) StageInRoom2 = 3;
		break;
	case 3:
		PaintRange( 3, 11, 47, 3, 1, 8, SPAAACE_CELL, 255 );
		if( gDirectionX < -.7 ) StageInRoom2 = 4;
		break;
	case 4:
		//Open hole in front of room.
		PaintRange( 3, 15, 47, 3, 2, 3, WHITE_BLOCK, 255 );
		ClearRange( 4, 15, 48, 1, 1, 1 );
		PaintRange( 4, 16, 48, 1, 1, 1, GOAL_BLOCK, 255 );
		SetWarpSpaceArea( 3, 15, 47, 4, 4, 3, .3, .3, .3 ); 
		if( gDirectionY < -.7 ) StageInRoom2 = 4;
		break;

	}
//printf( "%f %f %f %d\n", gDirectionX, gDirectionY, gDirectionZ, StageInRoom2 );

	if( IsPlayerInRange( 3, 15.1, 48, 3, 1, 2 ) )
	{
		room = 3;
	}
}

//Room with echo walls.
void RunRoom3()
{
	static int already_setup_jumpspace;
	static int already_removed_stretch = 0;
	static double TimeInRoom3;
	TimeInRoom3 += worldDeltaTime;
	gDaytime = 472;

	if( IsPlayerInRange( 2, 17, 41, 10, 10, 1.1 ) && !already_removed_stretch )
	{
		printf( "Dewarpify.\n" );
		//Remove the stretchyness that let us into this room.
		SetWarpSpaceArea( 1, 15, 40, 12, 12, 12, 1,1,1 );

		PaintRange( 4, 16, 48, 1, 1, 1, DEADGOAL_BLOCK, 255 );
		already_removed_stretch = 1;
	}

	if( firstrun )
	{
		GameTimer = 100;
		already_removed_stretch = 0;
		already_setup_jumpspace = 0;
		TimeInRoom3 = 0;
		firstrun = 0;
		SetWarpSpaceArea( 1, 15, 40, 12, 12, 12, .3, .3, .3 );
		PaintRange( 3, 15, 47, 3, 2, 3, WHITE_BLOCK, 255 );

		MakeEmptyBox( 1, 16, 40, 12, 12, 12, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Open area
		PaintRange( 6, 21, 46, 2, 2, 1, 17, 255 ); //Random little platform
		MakeEmptyBox( 3, 28, 47, 6, 5, 3, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Tucked away (For goal)
		ClearRange( 4, 28, 48, 5, 2, 2 ); //Open wall between open area and goal.
//		MakeJumpSection( 1, 16, 39, 12, 12, 3, 0, 0, 0 );
//		MakeJumpSection( 1, 16, 51, 12, 12, 3, 0, 0, 0 );
		PaintRange( 12, 17, 40, 1, 1, 1, GOAL_BLOCK, 255 );

		PaintRange( 6, 33, 47, 1, 1, 3, GOAL_BLOCK, 255 );

		ClearRange( 4, 15, 48, 1, 1, 1 );
	}

	if( IsPlayerInRange( 12, 17, 41, 1, 1, 2 ) && already_setup_jumpspace == 0 )
	{
		ClearRange( 1, 16, 37, 1, 1, 4 );
		ClearRange( 1, 16, 48, 1, 1, 4 );
		MakeJumpSection( 1, 16, 39, 12, 12, 3, 0, 0, 7 );
		MakeJumpSection( 1, 16, 51, 12, 12, 3, 0, 0, -7 );
		printf( "Jumpspace\n" );
		already_setup_jumpspace = 1;
	}

	int capden = 255 - TimeInRoom3 * 200;
	float warp = ((TimeInRoom3*.1)>1)?1:((TimeInRoom3*.1)+.1);

	if( capden < 0 )
	{
		if( already_removed_stretch == 0 )
			PaintRange( 4, 16, 48, 1, 1, 1, GOAL_BLOCK, 0 );
	}
	else
	{
		PaintRange( 4, 16, 48, 1, 1, 1, GOAL_BLOCK, capden );
	}

	if( IsPlayerInRange( 6, 32, 48, 1, 1, 2 ) )
	{
		room = 4;
	}
}

void RunRoom4()
{
	int x, y;

	//Make evil deadly floor room.
	static double TimeInRoom4;
	TimeInRoom4 += worldDeltaTime;
	if( firstrun )
	{
		GameTimer = 100;

		SetWarpSpaceArea( 2, 34, 42, 17, 15, 20, .2, .2, .4 );    //very flat big box.
		MakeEmptyBox( 3, 33, 43, 15, 15, 13, 2, 255, DEFAULT_BRIGHT, 1 ); //Main room.
		MakeEmptyBox( 3, 28, 47, 6, 5, 3, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Tucked away (For start point)
		PaintRange( 2, 32, 43+12, 17, 17, 3, SPAAACE_CELL, DEFAULT_DENSITY ); //Space on ceiling.

		//Lava walls
		PaintRange( 3, 35, 43, 1, 10, 2, 10, DEFAULT_DENSITY );
		PaintRange( 18, 35, 43, 1, 10, 2, 10, DEFAULT_DENSITY );

		//Fill some lava in in a random pattern.
		unsigned long  i = 1;
		for( x = 0; x < 15; x++ )
		for( y = 0; y < 11; y++ )
		{
			i = ( i * 2089 ) % 491;
			if( ( (i % 7) > 2 ) )
				QuickCell( 0, 3+x, 35+y, 43, 1, DEFAULT_BRIGHT, 60, 10 ); //Lava
		}

		TimeInRoom4 = 0;
		firstrun = 0;

	}

	int capden = 255 - TimeInRoom4 * 200;
	float warp = ((TimeInRoom4*.1)>1)?1:((TimeInRoom4*.1)+.1);

	//Open Room 3 goal to room 4.
	if( capden < 0 )
	{
		PaintRange( 6, 33, 48, 1, 1, 2, GOAL_BLOCK, 0 );
		PaintRange( 3, 28, 48, 6, 1, 2, DEADGOAL_BLOCK, 255 );
	}
	else
	{
		PaintRange( 6, 33, 48, 1, 1, 2, GOAL_BLOCK, capden );
		PaintRange( 3, 28, 48, 6, 1, 2, DEADGOAL_BLOCK, 255-capden );
	}

	for( x = 0; x < 15; x++ )
	for( y = 0; y < 15; y++ )
	{
		QuickCell( 0, 3+x, 33+y, 43+10, 1, DEFAULT_BRIGHT, (sin( x*2 + y*2 + TimeInRoom4)*80 + 80), 14 ); //Gold ore
	}
	UpdateZone( 3, 33, 43+10, 15, 15, 3 );

}

///XXX TODO: pointers should come in here.
void collision( struct CollisionProbe * ddat )
{
	printf( "CC %f %f %f  (%f)\n", ddat->TargetLocation.r, ddat->TargetLocation.g, ddat->TargetLocation.b, ddat->Normal.a ); 
}

void Update()
{
	if( gOverallUpdateNo == 0 )
	{
		initialize( );
	}

	if( room != lastroom )
	{
		firstrun = 1;
		lastroom = room;
	}

	gDaytime += worldDeltaTime;
	GameTimer -= worldDeltaTime;
	if( GameTimer < 0 )
	{
		StartAtRoom( room );
		GameAttempt++;
	}

	switch( room )
	{
	case 0: RunRoom0(); break;
	case 1: RunRoom1(); break;
	case 2: RunRoom2(); break;
	case 3: RunRoom3(); break;
	case 4: RunRoom4(); break;
	}


	sprintf( gDialog, "Room: %d\n", room );


	//Press 'r' to reset room.
	if( gKeyMap['r'] || gKeyMap['R'] )
	{
		StartAtRoom( room );
	}

//printf( "%f %f %f\n", gPositionX, gPositionY, gPositionZ );

	

/*
	totaltime += worldDeltaTime;
	sprintf( gDialog, "Update %f %f %f\n", gTargetHitX, gTargetHitY, gTargetHitZ );
	if( gMouseLastClickButton != -1 ) {
		if( gMouseLastClickButton == 0 )
		{
			//Left-mouse
			double targetx = gTargetHitX + gTargetNormalX*.5;
			double targety = gTargetHitY + gTargetNormalY*.5;
			double targetz = gTargetHitZ + gTargetNormalZ*.5;
			ChangeCell( 0, targetx, targety, targetz, 255, 255, 255, 10 );
		}
		else if( gMouseLastClickButton == 2 )
		{
			//Left-mouse
			double targetx = gTargetHitX - gTargetNormalX*.5;
			double targety = gTargetHitY - gTargetNormalY*.5;
			double targetz = gTargetHitZ - gTargetNormalZ*.5;
			ChangeCell( 0, targetx, targety, targetz, 0, 255, 0, 0 );
		}
		gMouseLastClickButton = -1;
	}
*/

}
