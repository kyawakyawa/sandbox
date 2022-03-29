/*
The MIT License (MIT)

Copyright (c) 2017 Eric Arnebäck

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define WIDTH 3200
#define HEIGHT 2400
#define WORKGROUP_SIZE 32

typedef struct {
  float4 value;
} Pixel;

__attribute__((reqd_work_group_size(WORKGROUP_SIZE, WORKGROUP_SIZE, 1)))
__kernel void
test(__global Pixel* outputs) {
  const uint index_x = get_global_id(0);
  const uint index_y = get_global_id(1);

  const float x = (float)index_x / (float)WIDTH;
  const float y = (float)index_y / (float)HEIGHT;

  float2 uv = (float2)(x, y);
  float n   = 0.0f;
  float2 c  = (float2)(-.445f, 0.0f) +
             (uv - (float2)(0.5f, 0.5f)) * (float2)(2.0f + 1.7f * 0.2f);
  float2 z    = (float2)(0.0f);
  const int M = 128.f;
  for (int i = 0; i < M; i++) {
    z = (float2)(z.x * z.x - z.y * z.y, 2.f * z.x * z.y) + c;
    if (dot(z, z) > 2.f) break;
    n++;
  }

  float t  = (float)n / M;
  float3 d = (float3)(0.3f, 0.3f, 0.5f);
  float3 e = (float3)(-0.2f, -0.3f, -0.5f);
  float3 f = (float3)(2.1f, 2.0f, 3.0f);
  float3 g = (float3)(0.0f, 0.1f, 0.0f);

  float4 color = (float4)(d + e * cos((float3)(6.28318f) * (f * t + g)), 1.0f);

  outputs[WIDTH * index_y + index_x].value = color;
}
