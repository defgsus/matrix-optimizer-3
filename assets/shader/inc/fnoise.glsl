// needs noise functions defined

// ----------------------- 2d fractal noise ---------------------

float fnoise1(
        in vec2 x,
        int num = 7,
        in float amp = 0.35,
        in vec2 scale = vec2(2.),
        in vec2 offset = vec2(1.)
    )
{
    num = max(2, num);
    float n = 0., a = .5;
    for (int i=0; i<num; ++i)
    {
        n += a * noise1(x + offset * float(i));
        x = x * scale;
        a *= amp;
    }
    return n;
}

vec2 fnoise2(
        in vec2 x,
        int num = 7,
        in vec2 amp = vec2(0.35),
        in vec2 scale = vec2(2.),
        in vec2 offset = vec2(1.)
    )
{
    num = max(2, num);
    vec2 n = vec2(0.), a = vec2(.5);
    for (int i=0; i<num; ++i)
    {
        n += a * noise2(x + offset * float(i));
        x = x * scale;
        a *= amp;
    }
    return n;
}

// ----------------------- 3d fractal noise ---------------------

float fnoise1(
        in vec3 x,
        int num = 7,
        in float amp = 0.35,
        in vec3 scale = vec3(2.),
        in vec3 offset = vec3(1.)
    )
{
    num = max(2, num);
    float n = 0., a = .5;
    for (int i=0; i<num; ++i)
    {
        n += a * noise1(x + offset * float(i));
        x = x * scale;
        a *= amp;
    }
    return n;
}
