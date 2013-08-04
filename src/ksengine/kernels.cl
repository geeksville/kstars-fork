__kernel void applyMatrixInPlace(          float4  m1,
                                           float4  m2,
                                           float4  m3,
                                  __global float4 *vs)
{
    int gid = get_global_id(0);
    float4 v = vs[gid];
    // Our vectors are really 3d, so we just set the last coord to 0 
    float4 w = (float4)(dot(m1, v), /* First row of m */
                        dot(m2, v), /* Second row of m */
                        dot(m3, v), /* Third row of m */
                        0);
    vs[gid] = w;
}

/*
 * FIXME: is there a way to avoid this code duplication?
 */
__kernel void applyMatrix(          float4  m1,
                                    float4  m2,
                                    float4  m3,
                           __global float4 *vs,
                           __global float4 *ws)
{
    int gid = get_global_id(0);
    float4 v = vs[gid];
    // Our vectors are really 3d, so we just set the last coord to 0 
    float4 w = (float4)(dot(m1, v), /* First row of m */
                        dot(m2, v), /* Second row of m */
                        dot(m3, v), /* Third row of m */
                        0);
    ws[gid] = w;
}

__kernel void aberrate(          float   expRapidity,
                        __global float4 *vs)
{
    int gid = get_global_id(0);
    float4 v = vs[gid];
    if( v.y < 0 ) {
        float2 ab = expRapidity*(1/(1-v.y))*v.xz;
        float n = dot(ab,ab);
        float4 vnew = (1/(1+n))*(float4)(2*ab.lo,n-1,2*ab.hi,0);
        vs[gid] = vnew;
    } else {
        float2 ab = (1/expRapidity)*(1/(1+v.y))*v.xz;
        float n = dot(ab,ab);
        float4 vnew = (1/(1+n))*(float4)(2*ab.lo,-n+1,2*ab.hi,0);
        vs[gid] = vnew;
    }
}
        
