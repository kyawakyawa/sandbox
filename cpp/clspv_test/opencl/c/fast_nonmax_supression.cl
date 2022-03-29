// OpenCL port of the FAST corner detector.
// Copyright (C) 2014, Itseez Inc. See the license at http://opencv.org

inline int cornerScore(__global const uchar* img, int step)
{
    int k, tofs, v = img[0], a0 = 0, b0;
    int d[16];
    #define LOAD2(idx, ofs) \
        tofs = ofs; d[idx] = (short)(v - img[tofs]); d[idx+8] = (short)(v - img[-tofs])
    LOAD2(0, 3);
    LOAD2(1, -step+3);
    LOAD2(2, -step*2+2);
    LOAD2(3, -step*3+1);
    LOAD2(4, -step*3);
    LOAD2(5, -step*3-1);
    LOAD2(6, -step*2-2);
    LOAD2(7, -step-3);

    #pragma unroll
    for( k = 0; k < 16; k += 2 )
    {
        int a = min((int)d[(k+1)&15], (int)d[(k+2)&15]);
        a = min(a, (int)d[(k+3)&15]);
        a = min(a, (int)d[(k+4)&15]);
        a = min(a, (int)d[(k+5)&15]);
        a = min(a, (int)d[(k+6)&15]);
        a = min(a, (int)d[(k+7)&15]);
        a = min(a, (int)d[(k+8)&15]);
        a0 = max(a0, min(a, (int)d[k&15]));
        a0 = max(a0, min(a, (int)d[(k+9)&15]));
    }

    b0 = -a0;
    #pragma unroll
    for( k = 0; k < 16; k += 2 )
    {
        int b = max((int)d[(k+1)&15], (int)d[(k+2)&15]);
        b = max(b, (int)d[(k+3)&15]);
        b = max(b, (int)d[(k+4)&15]);
        b = max(b, (int)d[(k+5)&15]);
        b = max(b, (int)d[(k+6)&15]);
        b = max(b, (int)d[(k+7)&15]);
        b = max(b, (int)d[(k+8)&15]);

        b0 = min(b0, max(b, (int)d[k]));
        b0 = min(b0, max(b, (int)d[(k+9)&15]));
    }

    return -b0-1;
}

///////////////////////////////////////////////////////////////////////////
// nonmaxSupression

__kernel
void FAST_nonmaxSupression(
    __global const int* kp_in, volatile __global int* kp_out,
    __global const uchar * _img, int step, int img_offset,
    int rows, int cols, int counter, int max_keypoints)
{
    const int idx = get_global_id(0);

    if (idx < counter)
    {
        int x = kp_in[1 + 2*idx];
        int y = kp_in[2 + 2*idx];
        __global const uchar* img = _img + mad24(y, step, x + img_offset);

        int s = cornerScore(img, step);

        if( (x < 4 || s > cornerScore(img-1, step)) +
            (y < 4 || s > cornerScore(img-step, step)) != 2 )
            return;
        if( (x >= cols - 4 || s > cornerScore(img+1, step)) +
            (y >= rows - 4 || s > cornerScore(img+step, step)) +
            (x < 4 || y < 4 || s > cornerScore(img-step-1, step)) +
            (x >= cols - 4 || y < 4 || s > cornerScore(img-step+1, step)) +
            (x < 4 || y >= rows - 4 || s > cornerScore(img+step-1, step)) +
            (x >= cols - 4 || y >= rows - 4 || s > cornerScore(img+step+1, step)) == 6)
        {
            int new_idx = atomic_inc(kp_out);
            if( new_idx < max_keypoints )
            {
                kp_out[1 + 3*new_idx] = x;
                kp_out[2 + 3*new_idx] = y;
                kp_out[3 + 3*new_idx] = s;
            }
        }
    }
}
