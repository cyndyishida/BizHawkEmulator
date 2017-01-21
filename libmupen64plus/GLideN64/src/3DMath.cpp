#include <math.h>
#include "3DMath.h"

void MultMatrix(float m0[4][4], float m1[4][4], float dest[4][4])
{
	int i;
	for (i = 0; i < 4; i++)
	{
		dest[0][i] = m0[0][i]*m1[0][0] + m0[1][i]*m1[0][1] + m0[2][i]*m1[0][2] + m0[3][i]*m1[0][3];
		dest[1][i] = m0[0][i]*m1[1][0] + m0[1][i]*m1[1][1] + m0[2][i]*m1[1][2] + m0[3][i]*m1[1][3];
		dest[2][i] = m0[0][i]*m1[2][0] + m0[1][i]*m1[2][1] + m0[2][i]*m1[2][2] + m0[3][i]*m1[2][3];
		dest[3][i] = m0[3][i]*m1[3][3] + m0[2][i]*m1[3][2] + m0[1][i]*m1[3][1] + m0[0][i]*m1[3][0];
	}
}

void TransformVectorNormalize(float vec[3], float mtx[4][4])
{
	float len;

	float vres[3];
	vres[0] = mtx[0][0] * vec[0]
		   + mtx[1][0] * vec[1]
		   + mtx[2][0] * vec[2];
	vres[1] = mtx[0][1] * vec[0]
		   + mtx[1][1] * vec[1]
		   + mtx[2][1] * vec[2];
	vres[2] = mtx[0][2] * vec[0]
		   + mtx[1][2] * vec[1]
		   + mtx[2][2] * vec[2];
	memcpy(vec, vres, sizeof(float)*3);
	len = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
	if (len != 0.0)
	{
		len = sqrtf(len);
		vec[0] /= len;
		vec[1] /= len;
		vec[2] /= len;
	}
}

void Normalize(float v[3])
{
#ifdef WIN32_ASM
	__asm {
		mov		esi, dword ptr [v]
										//	ST(6)			ST(5)			ST(4)			ST(3)			ST(2)			ST(1)			ST
		fld		dword ptr [esi+08h]		//																									v2
		fld		dword ptr [esi+04h]		//																					v2				v1
		fld		dword ptr [esi]			//																	v2				v1				v0
		fld1							//													v2				v1				v0				1.0
		fld		ST(3)					//									v2				v1				v0				1.0				v2
		fmul	ST, ST					//									v2				v1				v0				1.0				v2*v2
		fld		ST(3)					//					v2				v1				v0				1.0				v2*v2			v1
		fmul	ST, ST					//					v2				v1				v0				1.0				v2*v2			v1*v1
		fld		ST(3)					//	v2				v1				v0				1.0				v2*v2			v1*v1			v0
		fmul	ST, ST					//	v2				v1				v0				1.0				v2*v2			v1*v1			v0*v0
		fadd							//					v2				v1				v0				1.0				v2*v2			v1*v1+v0*v0
		fadd							//									v2				v1				v0				1.0				v2*v2+v1*v1+v0*v0
		ftst							// Compare ST to 0
		fstsw	ax						// Store FPU status word in ax
		sahf							// Transfer ax to flags register
		jz		End						// Skip if length is zero
		fsqrt							//									v2				v1				v0				1.0				len
		fdiv							//													v2				v1				v0				1.0/len
		fmul	ST(3), ST				//													v2*(1.0/len)	v1				v0				1.0/len
		fmul	ST(2), ST				//													v2*(1.0/len)	v1*(1.0/len)	v0				1.0/len
		fmul							//																	v2*(1.0/len)	v1*(1.0/len)	v0*(1.0/len)
		fstp	dword ptr [esi]			//																					v2*(1.0/len)	v1*(1.0/len)
		fstp	dword ptr [esi+04h]		//																									v2*(1.0/len)
		fstp	dword ptr [esi+08h]		//
End:
		finit
	}
#else // WIN32_ASM
	float len;

	len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	if (len != 0.0)	{
		len = sqrtf( len );
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
	}
#endif // WIN32_ASM
}


float DotProduct(const float v0[3], const float v1[3])
{
	float	dot;
#ifdef WIN32_ASM
	__asm {
		mov		esi, dword ptr [v0]
		mov		edi, dword ptr [v1]
		lea		ebx, [dot]

		fld		dword ptr [esi]
		fmul	dword ptr [edi]
		fld		dword ptr [esi+04h]
		fmul	dword ptr [edi+04h]
		fld		dword ptr [esi+08h]
		fmul	dword ptr [edi+08h]
		fadd
		fadd
		fstp	dword ptr [ebx]
	}
#else // WIN32_ASM
	dot = v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
#endif // WIN32_ASM
	return dot;
}
