#version 120

varying vec4 globalColor;
uniform sampler2D tex0;

varying vec2 texCoordVarying;
void main()
{
    float tex = texture2D(tex0, texCoordVarying);
    // globalColor is being set inside the vertex shader.
    // the fragment color is then set to the globalColor.

    if(tex.r > .5){
        tex.r = 254./255.;
        tex.g = 185./255.;
        tex.b = 24./255.;
    }
    gl_FragColor = tex;
}
