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
