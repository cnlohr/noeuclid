varying vec3 RayDirection;
varying vec3 InitialCamera;
varying vec2 PosInTex;
void main()
{
	InitialCamera = -(( gl_ModelViewMatrixInverse * vec4(0.0,0.0,0.0,-1.0) )).xyz;

	RayDirection = vec3( ( gl_Vertex  ).x, -( gl_Vertex  ).y, -2.0 );
	RayDirection = -( ( gl_ModelViewMatrixInverse * vec4(-RayDirection,0.0) ) ).xyz;

	//Careful -- don't do any transforms or anything, we want this triangle to take p the whole view.
	gl_Position = (gl_MultiTexCoord0*2.0-1.0) * vec4( 1., -1., 1., 1. );

	PosInTex = gl_MultiTexCoord0.xy;
}
