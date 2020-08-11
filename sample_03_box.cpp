#include <iostream>

#include "RenderHelp.h"

// 定义顶点结构
struct VertexAttrib { Vec3f pos; Vec2f uv; Vec3f color; };

// 顶点着色器输入
VertexAttrib vs_input[3];

// 模型
VertexAttrib mesh[] = {
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
const int VARYING_TEXUV = 0;
const int VARYING_COLOR = 1;

void draw_plane(RenderHelp& rh, int a, int b, int c, int d) 
{
	mesh[a].uv.x = 0, mesh[a].uv.y = 0, mesh[b].uv.x = 0, mesh[b].uv.y = 1;
	mesh[c].uv.x = 1, mesh[c].uv.y = 1, mesh[d].uv.x = 1, mesh[d].uv.y = 0;

	vs_input[0] = mesh[a];
	vs_input[1] = mesh[b];
	vs_input[2] = mesh[c];
	rh.DrawPrimitive();

	vs_input[0] = mesh[c];
	vs_input[1] = mesh[d];
	vs_input[2] = mesh[a];
	rh.DrawPrimitive();
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
	rh.SetVertexShader([&] (int index, ShaderContext& output) -> Vec4f {
			Vec4f pos = vs_input[index].pos.xyz1() * mat_mvp;  // 扩充成四维矢量并变换
			output.varying_vec2f[VARYING_TEXUV] = vs_input[index].uv;
			output.varying_vec4f[VARYING_COLOR] = vs_input[index].color.xyz1();
			return pos;
		});

	// 像素着色器
	rh.SetPixelShader([&] (ShaderContext& input) -> Vec4f {
			Vec2f coord = input.varying_vec2f[VARYING_TEXUV];	// 取得纹理坐标
			Vec4f tc = texture.Sample2D(coord);		// 纹理采样并返回像素颜色
			return tc;		// 返回纹理
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


