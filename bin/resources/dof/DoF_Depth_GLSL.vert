// "Depth of Field" demo for Ogre
// Copyright (C) 2006  Christian Lindequist Larsen
//
// This code is in the public domain. You may do whatever you want with it.

varying float depth; // in view space
varying vec3 norm;
varying vec3 vpos;
void main()
{
	vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
	depth = -viewPos.z;
    norm =  gl_NormalMatrix * gl_Normal;
    vpos = viewPos;
	gl_Position = ftransform(); 
	
}
