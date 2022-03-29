// OpenCL port of the FAST corner detector.
// Copyright (C) 2014, Itseez Inc. See the license at http://opencv.org

__kernel
void FAST_findKeypoints(
    __global const uchar * _img, int step, int img_offset,
    int img_rows, int img_cols,
    volatile __global int* kp_loc,
    int max_keypoints, int threshold )
{
    int j = (int)get_global_id(0) + 3;
    int i = (int)get_global_id(1) + 3;

    if (i < img_rows - 3 && j < img_cols - 3)
    {
        __global const uchar* img = _img + mad24(i, step, j + img_offset);
        int v = img[0], t0 = v - threshold, t1 = v + threshold;
        int k, tofs, v0, v1;
        int m0 = 0, m1 = 0;

        #define UPDATE_MASK(idx, ofs) \
            tofs = ofs; v0 = img[tofs]; v1 = img[-tofs]; \
            m0 |= ((v0 < t0) << idx) | ((v1 < t0) << (8 + idx)); \
            m1 |= ((v0 > t1) << idx) | ((v1 > t1) << (8 + idx))

        UPDATE_MASK(0, 3);
        if( (m0 | m1) == 0 )
            return;

        UPDATE_MASK(2, -step*2+2);
        UPDATE_MASK(4, -step*3);
        UPDATE_MASK(6, -step*2-2);

        #define EVEN_MASK (1+4+16+64)

        if( ((m0 | (m0 >> 8)) & EVEN_MASK) != EVEN_MASK &&
            ((m1 | (m1 >> 8)) & EVEN_MASK) != EVEN_MASK )
            return;

        UPDATE_MASK(1, -step+3);
        UPDATE_MASK(3, -step*3+1);
        UPDATE_MASK(5, -step*3-1);
        UPDATE_MASK(7, -step-3);
        if( ((m0 | (m0 >> 8)) & 255) != 255 &&
            ((m1 | (m1 >> 8)) & 255) != 255 )
            return;

        m0 |= m0 << 16;
        m1 |= m1 << 16;

        #define CHECK0(i) ((m0 & (511 << i)) == (511 << i))
        #define CHECK1(i) ((m1 & (511 << i)) == (511 << i))

        if( CHECK0(0) + CHECK0(1) + CHECK0(2) + CHECK0(3) +
            CHECK0(4) + CHECK0(5) + CHECK0(6) + CHECK0(7) +
            CHECK0(8) + CHECK0(9) + CHECK0(10) + CHECK0(11) +
            CHECK0(12) + CHECK0(13) + CHECK0(14) + CHECK0(15) +

            CHECK1(0) + CHECK1(1) + CHECK1(2) + CHECK1(3) +
            CHECK1(4) + CHECK1(5) + CHECK1(6) + CHECK1(7) +
            CHECK1(8) + CHECK1(9) + CHECK1(10) + CHECK1(11) +
            CHECK1(12) + CHECK1(13) + CHECK1(14) + CHECK1(15) == 0 )
            return;

        {
            int idx = atomic_inc(kp_loc);
            if( idx < max_keypoints )
            {
                kp_loc[1 + 2*idx] = j;
                kp_loc[2 + 2*idx] = i;
            }
        }
    }
}

