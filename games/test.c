#include "scripthelpers.h"


int firstrun = 0;
int lastroom = -1;
int room = 0;

//Only called once.
void initialize( )
{
	printf( "Initialize.\n" );

//Initial start toom is 0	
#define START_ROOM 0

#if START_ROOM == 1
	room = 1;
	gPositionX = 4.5;
	gPositionY = 7.92;
	gPositionZ = 51.92;
#elif START_ROOM == 3
	room = 3;
	firstrun = 1;
	gPositionX = 4.5;
	gPositionY = 15.29;
	gPositionZ = 48.44;
#elif START_ROOM == 4
	room = 4;
	firstrun = 1;
	gPositionX = 6.5;
	gPositionY = 31.5;
	gPositionZ = 49;

#else
	room = 0;
	gPositionX = 3;
	gPositionY = 3;
	gPositionZ = 64;
#endif

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
		if( gDirectionY > .5 && IsPlayerInRange( 3, 12, 48, 3, 3, 3 ) ) StageInRoom2 = 1;
		break;
	case 1:
		if( gDirectionY < -.5 ) StageInRoom2 = 2;
		break;
	case 2:
		if( gDirectionY > .5 ) StageInRoom2 = 3;
		break;
	case 3:
		PaintRange( 3, 11, 47, 3, 1, 8, SPAAACE_CELL, 255 );
		if( gDirectionY < -.5 ) StageInRoom2 = 4;
		break;
	case 4:
		//Open hole in front of room.
		PaintRange( 3, 15, 47, 3, 2, 3, WHITE_BLOCK, 255 );
		ClearRange( 4, 15, 48, 1, 1, 1 );
		PaintRange( 4, 16, 48, 1, 1, 1, GOAL_BLOCK, 255 );
		SetWarpSpaceArea( 3, 15, 47, 4, 4, 3, .3, .3, .3 ); 
		if( gDirectionY < -.5 ) StageInRoom2 = 4;
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
		already_removed_stretch = 0;
		already_setup_jumpspace = 0;
		TimeInRoom3 = 0;
		firstrun = 0;
		SetWarpSpaceArea( 1, 15, 40, 12, 12, 12, .3, .3, .3 );
		PaintRange( 3, 15, 47, 3, 2, 3, WHITE_BLOCK, 255 );

		MakeEmptyBox( 1, 16, 40, 12, 12, 12, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Open area
		MakeEmptyBox( 3, 28, 47, 6, 5, 3, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Tucked away (For goal)
		ClearRange( 4, 28, 48, 5, 2, 2 );
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
	static double TimeInRoom4;
	TimeInRoom4 += worldDeltaTime;
	if( firstrun )
	{
		MakeEmptyBox( 3, 28, 47, 6, 5, 3, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1 ); //Tucked away (For goal)
		TimeInRoom4 = 0;
		firstrun = 0;
	}

	int capden = 255 - TimeInRoom4 * 200;
	float warp = ((TimeInRoom4*.1)>1)?1:((TimeInRoom4*.1)+.1);

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


	switch( room )
	{
	case 0: RunRoom0(); break;
	case 1: RunRoom1(); break;
	case 2: RunRoom2(); break;
	case 3: RunRoom3(); break;
	case 4: RunRoom4(); break;
	}


	sprintf( gDialog, "Room: %d\n", room );

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
