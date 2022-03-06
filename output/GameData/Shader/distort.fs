#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;

// Output fragment color
out vec4 finalColor;

// I have no idea how this work i copied it from somewhere and already forgot the source
vec2 Distortion(in vec2 uv, in float k1, in float k2)
{
    uv = uv * 2.0 - 1.0;	// [-1:1]

    // positive values of K1 give barrel distortion, negative give pincushion
    float r2 = uv.x * uv.x + uv.y * uv.y;
    uv *= 1.0 + k1 * r2 + k2 * r2 * r2;
    
    uv = (uv * .5 + .5);	// restore -> [0:1]
    return uv;
}

void main()
{
    float k1 = 0.1;
    float k2 = 0.0;
    
    vec2 uv = Distortion(fragTexCoord, k1, k2);

    // using the distortion param as a scale factor, to keep the image close to the viewport dims
    float scale = 0.83;		
    
    uv = uv * scale - (scale * .5) + .5;	// scale from center

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, uv);

    // black the area outside of uv
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        texelColor = vec4(0.0);
    }

    // Multiply texel color with vertex color
    finalColor = texelColor * fragColor * colDiffuse;
}