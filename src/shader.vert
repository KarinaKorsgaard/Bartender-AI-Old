
#version 120
// these are for the programmable pipeline system
uniform mat4 modelViewProjectionMatrix;
attribute vec4 position;
attribute vec2 texcoord;

// this is something we're creating for this shader
varying vec2 texCoordVarying;

void main()
{
    vec2 texcoord = gl_MultiTexCoord0.xy;
    
    // here we move the texture coordinates
    texCoordVarying = vec2(texcoord.x, texcoord.y);
    
    // send the vertices to the fragment shader
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
