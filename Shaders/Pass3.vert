varying vec3 RayDirection;
varying vec3 InitialCamera;
varying vec2 PosInTex;

varying mat4 toss;
varying vec4 BasePosition;

void main()
{
	InitialCamera = -(( gl_ModelViewMatrixInverse * vec4(0.0,0.0,0.0,-1.0) )).xyz;

	RayDirection = vec3( ( gl_Vertex  ).x, -( gl_Vertex  ).y, -2.0 );
	BasePosition = vec4( RayDirection, 0. );
	RayDirection = -( ( gl_ModelViewMatrixInverse * vec4(-RayDirection,0.0) ) ).xyz;

	//Careful -- don't do any transforms or anything, we want this triangle to take p the whole view.
	gl_Position = (gl_MultiTexCoord0*2.0-1.0) * vec4( 1., -1., 1., 1. );

	PosInTex = gl_MultiTexCoord0.xy;
	toss = gl_ModelViewMatrix;
}
