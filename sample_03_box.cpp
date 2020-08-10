#include <iostream>

#include "RenderHelp.h"


struct { Vec3f pos; Vec2f uv; Vec3f color; } mesh[] = {
	{ {  1, -1,  1, }, { 0, 0 }, { 1.0f, 0.2f, 0.2f }, },
	{ { -1, -1,  1, }, { 0, 1 }, { 0.2f, 1.0f, 0.2f }, },
	{ { -1,  1,  1, }, { 1, 1 }, { 0.2f, 0.2f, 1.0f }, },
	{ {  1,  1,  1, }, { 1, 0 }, { 1.0f, 0.2f, 1.0f }, },
	{ {  1, -1, -1, }, { 0, 0 }, { 1.0f, 1.0f, 0.2f }, },
	{ { -1, -1, -1, }, { 0, 1 }, { 0.2f, 1.0f, 1.0f }, },
	{ { -1,  1, -1, }, { 1, 1 }, { 1.0f, 0.3f, 0.3f }, },
	{ {  1,  1, -1, }, { 1, 0 }, { 0.2f, 1.0f, 0.3f }, },
};

// 定义属性和 varying 中的纹理坐标 key
const int ATTR_TEXUV = 0;
const int ATTR_COLOR = 1;
const int VARYING_TEXUV = 0;
const int VARYING_COLOR = 1;

void draw_triangle(RenderHelp& rh, int a, int b, int c) 
{
	rh.SetVertex(0, mesh[a].pos);
	rh.SetVertex(1, mesh[b].pos);
	rh.SetVertex(2, mesh[c].pos);
	rh.SetAttrib(0, ATTR_TEXUV, mesh[a].uv);
	rh.SetAttrib(1, ATTR_TEXUV, mesh[b].uv);
	rh.SetAttrib(2, ATTR_TEXUV, mesh[c].uv);
	rh.SetAttrib(0, ATTR_COLOR, mesh[a].color);
	rh.SetAttrib(1, ATTR_COLOR, mesh[b].color);
	rh.SetAttrib(2, ATTR_COLOR, mesh[c].color);
	
	rh.DrawPrimitive();
}

void draw_plane(RenderHelp& rh, int a, int b, int c, int d) 
{
	mesh[a].uv.x = 0, mesh[a].uv.y = 0, mesh[b].uv.x = 0, mesh[b].uv.y = 1;
	mesh[c].uv.x = 1, mesh[c].uv.y = 1, mesh[d].uv.x = 1, mesh[d].uv.y = 0;
	draw_triangle(rh, a, b, c);
	draw_triangle(rh, c, d, a);
}

void draw_box(RenderHelp& rh) 
{
}

int main(void)
{
	RenderHelp rh(800, 600);

	// 定义一个纹理，并生成网格图案
	Bitmap texture(256, 256);
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 256; x++) {
			int k = (x / 32 + y / 32) & 1;
			texture.SetPixel(x, y, k? 0xffffffff : 0xff3fbcef);
		}
	}

	// 定义变换矩阵：模型变换，摄像机变换，透视变换
	Mat4x4f mat_model = matrix_set_rotate(-1, -0.5, 1, 1);	// 模型变换，旋转一定角度
	Mat4x4f mat_view = matrix_set_lookat({3.5, 0, 0}, {0,0,0}, {0,0,1});	// 摄像机方位
	Mat4x4f mat_proj = matrix_set_perspective(3.1415926f * 0.5f, 800 / 600.0, 1.0, 500.0f);
	Mat4x4f mat_mvp = mat_model * mat_view * mat_proj;	// 综合变换矩阵

	// 顶点着色器
	rh.SetVertexShader([&] (VS_Input& input, PS_Input& output) {
			output.pos = input.pos * mat_mvp;	// 输出变换后的坐标
			output.varying_vec2f[VARYING_TEXUV] = input.attrib_vec2f[ATTR_TEXUV];
			output.varying_vec4f[VARYING_COLOR] = input.attrib_vec3f[ATTR_COLOR].xyz1();
			// std::cout << "color: " << input.attrib_vec4f[ATTR_COLOR] << "\n";
		});

	// 像素着色器
	rh.SetPixelShader([&] (PS_Input& input) -> Vec4f {
			Vec2f coord = input.varying_vec2f[VARYING_TEXUV];	// 取得纹理坐标
			Vec4f tc = texture.Sample2D(coord);		// 纹理采样并返回像素颜色
		#if 1
			return tc;		// 返回纹理
		#else
			Vec4f cc = input.varying_vec4f[VARYING_COLOR];
			return tc * cc;	// 纹理混合颜色
		#endif
		});

	// 绘制盒子
	draw_plane(rh, 0, 1, 2, 3);
	draw_plane(rh, 7, 6, 5, 4);
	draw_plane(rh, 0, 4, 5, 1);
	draw_plane(rh, 1, 5, 6, 2);
	draw_plane(rh, 2, 6, 7, 3);
	draw_plane(rh, 3, 7, 4, 0);

	// 保存结果
	rh.SaveFile("output.bmp");

	// 用画板显示图片
#if defined(_WIN32) || defined(WIN32)
	system("mspaint.exe output.bmp");
#endif

	return 0;
}


