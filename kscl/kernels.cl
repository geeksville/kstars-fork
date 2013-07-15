#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void applyMatrix(          double16 m,
                           __global double4* as, 
                           __global double4* bs )
{
    int gid = get_global_id(0);
    double4 a = as[gid];
    // Our vectors are really 3d, so we just set the last coord to 0 
    double4 b = (double4)(dot(m.lo.lo, a), /* First row of m */
                          dot(m.lo.hi, a), /* Second row of m */
                          dot(m.hi.lo, a), /* Third row of m */
                          0);
    bs[gid] = b;
}
