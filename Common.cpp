#include "Common.h"

void cross3d(float * out, const float * a, const float * b) {
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

void normalize3d(float * out, const float * in) {
    float r = 1. / sqrtf(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
    out[0] = in[0] * r;
    out[1] = in[1] * r;
    out[2] = in[2] * r;
}

float dot3d(const float * a, const float * b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void copy3d(float * out, const float * in) {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}



/////////////////////////////////////QUATERNIONS//////////////////////////////////////////
//Originally from Mercury (Copyright (C) 2009 by Joshua Allen, Charles Lohr, Adam Lowman)
//Under the mit/X11 license.

void quatcopy(float * qout, const float * qin) {
    qout[0] = qin[0];
    qout[1] = qin[1];
    qout[2] = qin[2];
    qout[3] = qin[3];
}

float quatinvsqmagnitude(const float * q) {
    return 1. / ((q[0] * q[0])+(q[1] * q[1])+(q[2] * q[2])+(q[3] * q[3]));
}
void quatscale(float * qout, const float * qin, float s);
void quatnormalize(float * qout, const float * qin) {
    float imag = quatinvsqmagnitude(qin);
    quatscale(qout, qin, imag);
}

void quatfromeuler(float * q, const float * euler) {
    float X = euler[0] / 2.0f; //roll
    float Y = euler[1] / 2.0f; //pitch
    float Z = euler[2] / 2.0f; //yaw

    float cx = cos(X);
    float sx = sin(X);
    float cy = cos(Y);
    float sy = sin(Y);
    float cz = cos(Z);
    float sz = sin(Z);

    //Correct according to
    //http://en.wikipedia.org/wiki/Conversion_between_Quaternions_and_Euler_angles
    q[0] = cx * cy * cz + sx * sy*sz; //q1
    q[1] = sx * cy * cz - cx * sy*sz; //q2
    q[2] = cx * sy * cz + sx * cy*sz; //q3
    q[3] = cx * cy * sz - sx * sy*cz; //q4

    quatnormalize(q, q);
}

void quatfromaxisangle(float * q, const float * axis, float radians) {
    float v[3];
    normalize3d(v, axis);

    float sn = sin(radians / 2.0f);
    q[0] = cos(radians / 2.0f);
    q[1] = sn * v[0];
    q[2] = sn * v[1];
    q[3] = sn * v[2];

    quatnormalize(q, q);
}

void quattomatrix(float * matrix44, const float * qin) {
    float q[4];
    quatnormalize(q, qin);

    //Reduced calulation for speed
    float xx = 2 * q[1] * q[1];
    float xy = 2 * q[1] * q[2];
    float xz = 2 * q[1] * q[3];
    float xw = 2 * q[1] * q[0];

    float yy = 2 * q[2] * q[2];
    float yz = 2 * q[2] * q[3];
    float yw = 2 * q[2] * q[0];

    float zz = 2 * q[3] * q[3];
    float zw = 2 * q[3] * q[0];

    //opengl major
    matrix44[0] = 1 - yy - zz;
    matrix44[1] = xy - zw;
    matrix44[2] = xz + yw;
    matrix44[3] = 0;

    matrix44[4] = xy + zw;
    matrix44[5] = 1 - xx - zz;
    matrix44[6] = yz - xw;
    matrix44[7] = 0;

    matrix44[8] = xz - yw;
    matrix44[9] = yz + xw;
    matrix44[10] = 1 - xx - yy;
    matrix44[11] = 0;

    matrix44[12] = 0;
    matrix44[13] = 0;
    matrix44[14] = 0;
    matrix44[15] = 1;
}

void quatgetconjugate(float * qout, const float * qin) {
    qout[0] = qin[0];
    qout[1] = -qin[1];
    qout[2] = -qin[2];
    qout[3] = -qin[3];
}

void quatrotateabout(float * qout, const float * a, const float * b) {
    float q1[4];
    float q2[4];

    quatnormalize(q1, a);
    quatnormalize(q2, b);

    qout[0] = (q1[0] * q2[0])-(q1[1] * q2[1])-(q1[2] * q2[2])-(q1[3] * q2[3]);
    qout[1] = (q1[0] * q2[1])+(q1[1] * q2[0])+(q1[2] * q2[3])-(q1[3] * q2[2]);
    qout[2] = (q1[0] * q2[2])-(q1[1] * q2[3])+(q1[2] * q2[0])+(q1[3] * q2[1]);
    qout[3] = (q1[0] * q2[3])+(q1[1] * q2[2])-(q1[2] * q2[1])+(q1[3] * q2[0]);
}

void quatscale(float * qout, const float * qin, float s) {
    qout[0] = qin[0] * s;
    qout[1] = qin[1] * s;
    qout[2] = qin[2] * s;
    qout[3] = qin[3] * s;
}

void quatgetreciprocal(float * qout, const float * qin) {
    float m = quatinvsqmagnitude(qin);
    quatgetconjugate(qout, qin);
    quatscale(qout, qout, m);
}

Vec3f quatrotatevector(const float * quat, Vec3f v) {
    float tquat[4];
    float vquat[4];
    float qrecp[4];

    vquat[0] = (v.len2() < 0.001) ? 1 : 0; //XXX IS THIS RIGHT? IT LOOKS WRONG!!!
    vquat[1] = v.x;
    vquat[2] = v.y;
    vquat[3] = v.z;

    quatrotateabout(tquat, quat, vquat);
    quatgetreciprocal(qrecp, quat);
    quatrotateabout(vquat, tquat, qrecp);

    return {vquat[1],vquat[2],vquat[3]};
}

void quatrotatevector(float * vec3out, const float * quat, const float * vec3in) {
    float tquat[4];
    float vquat[4];
    float qrecp[4];

    vquat[0] = ((vec3in[0] * vec3in[0] + vec3in[1] * vec3in[1] + vec3in[2] * vec3in[2]) < 0.001) ? 1 : 0; //XXX IS THIS RIGHT? IT LOOKS WRONG!!!
    vquat[1] = vec3in[0];
    vquat[2] = vec3in[1];
    vquat[3] = vec3in[2];

    quatrotateabout(tquat, quat, vquat);
    quatgetreciprocal(qrecp, quat);
    quatrotateabout(vquat, tquat, qrecp);

    vec3out[0] = vquat[1];
    vec3out[1] = vquat[2];
    vec3out[2] = vquat[3];
}


//From: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
//From: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html

void quatfrommatrix(float * q, float * mat4) {
#define M4F(x,y) mat4[y*4+x]
#define SIGNx(x) (((x)<0)?-1:1)
    q[0] = (M4F(0, 0) + M4F(1, 1) + M4F(2, 2) + 1.0f) / 4.0f;
    q[1] = (M4F(0, 0) - M4F(1, 1) - M4F(2, 2) + 1.0f) / 4.0f;
    q[2] = (-M4F(0, 0) + M4F(1, 1) - M4F(2, 2) + 1.0f) / 4.0f;
    q[3] = (-M4F(0, 0) - M4F(1, 1) + M4F(2, 2) + 1.0f) / 4.0f;
    if (q[0] < 0.0f) q[0] = 0.0f;
    if (q[1] < 0.0f) q[1] = 0.0f;
    if (q[2] < 0.0f) q[2] = 0.0f;
    if (q[3] < 0.0f) q[3] = 0.0f;
    q[0] = sqrt(q[0]);
    q[1] = sqrt(q[1]);
    q[2] = sqrt(q[2]);
    q[3] = sqrt(q[3]);
    if (q[0] >= q[1] && q[0] >= q[2] && q[0] >= q[3]) {
        q[0] *= +1.0f;
        q[1] *= SIGNx(M4F(2, 1) - M4F(1, 2));
        q[2] *= SIGNx(M4F(0, 2) - M4F(2, 0));
        q[3] *= SIGNx(M4F(1, 0) - M4F(0, 1));
    } else if (q[1] >= q[0] && q[1] >= q[2] && q[1] >= q[3]) {
        q[0] *= SIGNx(M4F(2, 1) - M4F(1, 2));
        q[1] *= +1.0f;
        q[2] *= SIGNx(M4F(1, 0) + M4F(0, 1));
        q[3] *= SIGNx(M4F(0, 2) + M4F(2, 0));
    } else if (q[2] >= q[0] && q[2] >= q[1] && q[2] >= q[3]) {
        q[0] *= SIGNx(M4F(0, 2) - M4F(2, 0));
        q[1] *= SIGNx(M4F(1, 0) + M4F(0, 1));
        q[2] *= +1.0f;
        q[3] *= SIGNx(M4F(2, 1) + M4F(1, 2));
    } else if (q[3] >= q[0] && q[3] >= q[1] && q[3] >= q[2]) {
        q[0] *= SIGNx(M4F(1, 0) - M4F(0, 1));
        q[1] *= SIGNx(M4F(2, 0) + M4F(0, 2));
        q[2] *= SIGNx(M4F(2, 1) + M4F(1, 2));
        q[3] *= +1.0f;
    } else {
        //printf("coding error\n");
        q[0] = 1;
        q[1] = 0;
        q[2] = 0;
        q[3] = 0;
    }
    quatnormalize(q, q);
}
