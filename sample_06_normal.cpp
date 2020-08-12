#include "RenderHelp.h"
#include "Model.h"


int main(void) 
{
	RenderHelp rh(600, 800);

	// 加载模型
	Model model("res/diablo3_pose.obj");

	Vec3f eye_pos = {0, -0.5, 1.7};
	Vec3f eye_at = {0, 0, 0};
	Vec3f eye_up = {0, 1, 0};
	Vec3f light_dir = {1, 1, 0.85};			// 光的方向
	float perspective = 3.1415926f * 0.5f;

	Mat4x4f mat_model = matrix_set_scale(1, 1, 1);
	Mat4x4f mat_view = matrix_set_lookat(eye_pos, eye_at, eye_up);
	Mat4x4f mat_proj = matrix_set_perspective(perspective, 6 / 8.0, 1.0, 500.0f);
	Mat4x4f mat_mvp = mat_model * mat_view * mat_proj;

	// 用于将法向量从模型坐标系变换到世界坐标系
	Mat4x4f mat_model_it = matrix_invert(mat_model).Transpose();

	// 顶点属性
	struct { Vec3f pos; Vec3f normal; Vec2f uv; } vs_input[3];

	const int VARYING_UV = 0;

	rh.SetVertexShader([&] (int index, ShaderContext& output) -> Vec4f {
			Vec4f pos = vs_input[index].pos.xyz1() * mat_mvp;
			output.varying_vec2f[VARYING_UV] = vs_input[index].uv;
			return pos;
		});

	rh.SetPixelShader([&] (ShaderContext& input) -> Vec4f {
			Vec2f uv = input.varying_vec2f[VARYING_UV];
			// 归一化光的方向
			Vec3f l = vector_normalize(light_dir);
			// 从模型读取每个点的法向并变换到世界坐标系
			Vec3f n = (model.normal(uv).xyz1() * mat_model_it).xyz();
			// 点乘 n,l 获得光照强度，Saturate 可以对范围 [0,1] 裁剪
			float intense = Saturate(vector_dot(n, l) + 0.1f);
			Vec4f color = model.diffuse(uv);
			// 纹理乘以光的强度
			return color * intense;
		});

	// 迭代模型每一个面
	for (int i = 0; i < model.nfaces(); i++) {
		// 设置三个顶点的输入，供 VS 读取
		for (int j = 0; j < 3; j++) {
			vs_input[j].pos = model.vert(i, j);
			vs_input[j].uv = model.uv(i, j);
			vs_input[j].normal = model.normal(i, j);
		}
		// 绘制三角形
		rh.DrawPrimitive();
	}

	rh.SaveFile("output.bmp");

#if defined(WIN32) || defined(_WIN32)
	system("mspaint output.bmp");
#endif

	return 0;
}


