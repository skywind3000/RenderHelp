#include "RenderHelp.h"


int main(void)
{
	// 初始化渲染器和帧缓存大小
	RenderHelp rh(800, 600);

	const int ATTRIB_COLOR = 0;     // 定义一个属性的 key
	const int VARYING_COLOR = 0;    // 定义一个 varying 的 key

	// 顶点着色器，输出坐标，初始化 varying
	rh.SetVertexShader([&] (VS_Input& input, PS_Input& output) {
			output.pos = input.pos;		// 直接复制坐标
			output.varying_vec4f[VARYING_COLOR] = input.attrib_vec4f[ATTRIB_COLOR];
		});

	// 像素着色器，返回颜色
	rh.SetPixelShader([&] (PS_Input& input) -> Vec4f {
			return input.varying_vec4f[VARYING_COLOR];
		});

	// 设置三个顶点的坐标
	rh.SetVertex(0, { 0.0,  0.7, 0.90});
	rh.SetVertex(1, {-0.6, -0.2, 0.01});
	rh.SetVertex(2, {+0.6, -0.2, 0.01});
	
	// 设置三个顶点的颜色属性，属性可以有四种类型：float, Vec2f/3f/4f
	// 这里设置的是 Vec4f 的属性，故上面 PS 中用 attrib_vec4f 取出来
	// 不同属性的 KEY 用整数来区别，这里就是 ATTRIB_COLOR 这个整数
	rh.SetAttrib(0, ATTRIB_COLOR, Vec4f(1, 0, 0, 1.0f));
	rh.SetAttrib(1, ATTRIB_COLOR, Vec4f(0, 1, 0, 1.0f));
	rh.SetAttrib(2, ATTRIB_COLOR, Vec4f(0, 0, 1, 1.0f));

	// 渲染并保存
	rh.DrawPrimitive();
	rh.SaveFile("output.bmp");

	// 用画板显示图片
#if defined(_WIN32) || defined(WIN32)
	system("mspaint.exe output.bmp");
#endif

	return 0;
}


