
/** Constructs a matrix from v and it's computed orthogonal axis */
mat3 compute_orthogonals(in vec3 v)
{
    vec3 v1 = (abs(v.x) > abs(v.y)) ?
        vec3(-v.z, 0., v.x) : vec3(0., v.z, -v.y);
    vec3 v2 = cross(v, v1);

    return mat3(v, v1, v2);
}
