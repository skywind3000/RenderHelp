#include "RenderHelp.h"


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
	Mat4x4f mat_model = matrix_set_identity();	// 模型变换
	Mat4x4f mat_view = matrix_set_lookat({-0.7, 0, 1.5}, {0,0,0}, {0,0,1});	// 摄像机方位
	Mat4x4f mat_proj = matrix_set_perspective(3.1415926f * 0.5f, 800 / 600.0, 1.0, 500.0f);
	Mat4x4f mat_mvp = mat_model * mat_view * mat_proj;	// 综合变换矩阵

	// 定义顶点输入
	struct VertexAttrib { Vec4f pos; Vec2f texuv; } vs_input[3];

	// 定义属性和 varying 中的纹理坐标 key
	const int VARYING_TEXUV = 0;

	// 顶点着色器
	rh.SetVertexShader([&] (int index, ShaderContext& output) {
			Vec4f pos = vs_input[index].pos * mat_mvp;	// 输出变换后的坐标
			output.varying_vec2f[VARYING_TEXUV] = vs_input[index].texuv;
			return pos;
		});

	// 像素着色器
	rh.SetPixelShader([&] (ShaderContext& input) -> Vec4f {
			Vec2f coord = input.varying_vec2f[VARYING_TEXUV];	// 取得纹理坐标
			return texture.Sample2D(coord);		// 纹理采样并返回像素颜色
		});

	// 0 1
	// 3 2  绘制两个三角形，组成一个矩形
	VertexAttrib vertex[] = {
		{ { 1, -1, -1, 1}, {0, 0} },
		{ { 1,  1, -1, 1}, {1, 0} },
		{ {-1,  1, -1, 1}, {1, 1} },
		{ {-1, -1, -1, 1}, {0, 1} },
	};

	vs_input[0] = vertex[0];
	vs_input[1] = vertex[1];
	vs_input[2] = vertex[2];
	rh.DrawPrimitive();

	vs_input[0] = vertex[2];
	vs_input[1] = vertex[3];
	vs_input[2] = vertex[0];
	rh.DrawPrimitive();

	// 保存文件
	rh.SaveFile("output.bmp");

	// 用画板显示图片
#if defined(_WIN32) || defined(WIN32)
	system("mspaint.exe output.bmp");
#endif

	return 0;
}


