#version 120

varying vec4 globalColor;
uniform sampler2D tex0;

varying vec2 texCoordVarying;
void main()
{
    float tex = texture2D(tex0, texCoordVarying);
    // globalColor is being set inside the vertex shader.
    // the fragment color is then set to the globalColor.


    gl_FragColor = tex;
}
