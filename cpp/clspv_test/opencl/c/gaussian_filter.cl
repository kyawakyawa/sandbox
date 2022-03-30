#define WORKGROUP_SIZE 32

#define IN(x_, y_) (0 < (x_) && (x_) < (int)w && 0 < (y_) && (y_) < (int)h)

#define ADD(offset_x, offset_y)                                                \
  if (IN((int)index_x + (offset_x), (int)index_y + (offset_y))) {              \
    float v = (float)src[center_idx];                                          \
    const float offset_xf = (float)offset_x;                                   \
    const float offset_yf = (float)offset_y;                                   \
    v = norm_factor * inv_sigma *                                              \
        exp(-fma(offset_xf, offset_xf, offset_yf * offset_yf) *                \
            half_inv_simga2) *                                                 \
        v;                                                                     \
    dst[center_idx] += src[center_idx] = (uchar)v;                             \
  }

// TODO(anyone): Specify workgroup in host code.
__attribute__((reqd_work_group_size(WORKGROUP_SIZE, WORKGROUP_SIZE, 1)))
__kernel void
gaussian_filter3x3_glayscale(__global uchar *dst, __global uchar *src, uint w,
                             uint h, float sigma) {
  const uint index_x = get_global_id(0);
  const uint index_y = get_global_id(1);

  __constant const float norm_factor =
      0.39894228040143270286f; // 1 / sqrt(2 * pi)

  if (index_x >= w || index_y >= h)
    return;

  const float inv_sigma = 1.0f / sigma;
  const float half_inv_simga2 = 0.5f * inv_sigma * inv_sigma;
  const uint center_idx = index_y * w + index_x;

  dst[center_idx] = 0;

  ADD(-1, -1)
  ADD(0, -1)
  ADD(1, -1)

  ADD(-1, 0)
  ADD(0, 0)
  ADD(1, 0)

  ADD(-1, 1)
  ADD(0, 1)
  ADD(1, 1)
}
