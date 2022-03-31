#define WORKGROUP_SIZE 32

#define IN(x_, y_) (0 <= (x_) && (x_) < (int)w && 0 <= (y_) && (y_) < (int)h)

#define ADD(offset_x, offset_y)                                   \
  {                                                               \
    int x_id = (int)index_x + (offset_x);                         \
    int y_id = (int)index_y + (offset_y);                         \
    if (IN(x_id, y_id)) {                                         \
      float v               = ((float)src[y_id * w + x_id]);      \
      const float offset_xf = (float)offset_x;                    \
      const float offset_yf = (float)offset_y;                    \
      v *= norm_factor * inv_sigma *                              \
           exp(-(offset_xf * offset_xf + offset_yf * offset_yf) * \
               half_inv_simga2);                                  \
      sum += v;                                                   \
    }                                                             \
  }

#define ADD_RAW(offset_y) \
  ADD(-3, (offset_y))     \
  ADD(-2, (offset_y))     \
  ADD(-1, (offset_y))     \
  ADD(-0, (offset_y))     \
  ADD(1, (offset_y))      \
  ADD(2, (offset_y))      \
  ADD(3, (offset_y))

// TODO(anyone): Specify workgroup in host code.
__attribute__((reqd_work_group_size(WORKGROUP_SIZE, WORKGROUP_SIZE, 1)))
__kernel void
gaussian_filter5x5_glayscale(__global uchar *dst, __global const uchar *src,
                             uint w, uint h, float sigma) {
  const uint index_x = get_global_id(0);
  const uint index_y = get_global_id(1);

  // __constant const float norm_factor =
  //     0.39894228040143270286f;  // 1 / sqrt(2 * pi)

  __constant const float norm_factor = 0.15915494309189534561;  // 1 / 2 * pi

  if (index_x >= w || index_y >= h) return;

  const float inv_sigma       = 1.0f / sigma;
  const float half_inv_simga2 = 0.5f * inv_sigma * inv_sigma;
  const uint center_idx       = index_y * w + index_x;

  dst[center_idx] = 0;

  float sum = 0.f;

  ADD_RAW(-3);
  ADD_RAW(-2);
  ADD_RAW(-1);
  ADD_RAW(0);
  ADD_RAW(1);
  ADD_RAW(2);
  ADD_RAW(3);

  dst[center_idx] = (uchar)(sum);
}
