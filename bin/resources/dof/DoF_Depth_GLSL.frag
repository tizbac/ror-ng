// "Depth of Field" demo for Ogre
// Copyright (C) 2006  Christian Lindequist Larsen
//
// This code is in the public domain. You may do whatever you want with it.

// dofParams coefficients:
// x = near blur depth; y = focal plane depth; z = far blur depth
// w = blurriness cutoff constant for objects behind the focal plane
uniform vec4 dofParams;

varying float depth; // in view space
varying vec3 norm;
varying vec3 vpos;
void main()
{
	float f;
	
	if (depth < dofParams.y)
	{
		// scale depth value between near blur distance and focal distance to
		// [-1, 0] range
		f = (depth - dofParams.y) / (dofParams.y - dofParams.x);
	}
	else
	{
		// scale depth value between focal distance and far blur distance to
		// [0, 1] range
		f = (depth - dofParams.y) / (dofParams.z - dofParams.y);
		// clamp the far blur to a maximum blurriness
		f = clamp(f, 0.0, dofParams.w);
	}
    vec3 norma = normalize(norm);
	// scale and bias into [0, 1] range
	gl_FragData[0] = vec4(norma.x,norma.y,norma.z,0.5*f + 0.5);
	gl_FragData[1] = vec4(vpos.x,vpos.y,vpos.z,1);
	
}
