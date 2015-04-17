vec2 rotate(in vec2 v, float rad)
{
    float s = sin(rad), c = cos(rad);
    return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}
