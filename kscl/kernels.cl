#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void applyMatrix(          double16 m,
                           __global double4* vs)
{
    int gid = get_global_id(0);
    double4 v = vs[gid];
    // Our vectors are really 3d, so we just set the last coord to 0 
    double4 w = (double4)(dot(m.lo.lo, v), /* First row of m */
                          dot(m.lo.hi, v), /* Second row of m */
                          dot(m.hi.lo, v), /* Third row of m */
                          0);
    vs[gid] = w;
}
