#version 120

varying vec2 outputTexCoords;

uniform sampler2D myTexture;

void main()
{
	vec4 tex = texture2D(myTexture, outputTexCoords);
	gl_FragColor = tex;
}