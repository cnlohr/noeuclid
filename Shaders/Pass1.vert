varying vec3 RayDirection;
varying vec3 InitialCamera;
varying vec3 AuxRotation;

varying float maxdist;
varying float doPhysics;

void main()
{
	maxdist = gl_Vertex.z;
        
        AuxRotation = gl_MultiTexCoord1.xyz;

	if( gl_Color.a < 0. )
	{
		//We're in an override mode (used for physics, not regular stuff)
		InitialCamera = gl_Color.rgb;
		RayDirection = gl_Normal;
		doPhysics = 1.;
	}
	else
	{
		InitialCamera = -(( gl_ModelViewMatrixInverse * vec4(0.0,0.0,0.0,-1.0) )).xyz;
		RayDirection = vec3( ( gl_Vertex  ).x, ( gl_Vertex  ).y, -2.0 );
		RayDirection = -( ( gl_ModelViewMatrixInverse * vec4(-RayDirection,0.0) ) ).xyz;
		doPhysics = 0.0;
	}

	//Careful -- don't do any transforms or anything, we want this triangle to take p the whole view.
	gl_Position = (gl_MultiTexCoord0*2.0-1.0) * vec4( 1., -1., 1., 1. );
}
