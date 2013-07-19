#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void applyMatrix(          double4  m1,
                                    double4  m2,
                                    double4  m3,
                           __global double4 *vs)
{
    int gid = get_global_id(0);
    double4 v = vs[gid];
    // Our vectors are really 3d, so we just set the last coord to 0 
    double4 w = (double4)(dot(m1, v), /* First row of m */
                          dot(m2, v), /* Second row of m */
                          dot(m3, v), /* Third row of m */
                          0);
    vs[gid] = w;
}

__kernel void aberrate(          double   expRapidity,
                        __global double4 *vs)
{
    int gid = get_global_id(0);
    double4 v = vs[gid];
    if( v.y < 0 ) {
        double2 ab = expRapidity*(1/(1-v.y))*v.xz;
        double n = dot(ab,ab);
        double4 vnew = (1/(1+n))*(double4)(2*ab.lo,n-1,2*ab.hi,0);
        vs[gid] = vnew;
    } else {
        double2 ab = expRapidity*(1/(1+v.y))*v.xz;
        double n = dot(ab,ab);
        double4 vnew = (1/(1+n))*(double4)(2*ab.lo,-n+1,2*ab.hi,0);
        vs[gid] = vnew;
    }
}
        
