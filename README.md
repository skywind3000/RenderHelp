# RenderHelp

可编程渲染管线实现，帮助初学者学习渲染原理。

## 特性介绍

- 单个 RenderHelp.h 文件，从画点开始实现可编程渲染管线，无外部依赖。
- 模型标准，计算精确，使用类 Direct3D 接口。
- 包含一套完整的矢量/矩阵库。
- 包含一套位图 Bitmap 库，方便画点、画线、加载纹理、纹理采样等。
- 使用 C++ 编写顶点着色器 (Vertex Shader) 和像素着色器 (Pixel Shader)，方便断点和调试。
- 使用 Edge Equation 精确计算三角形覆盖范围，处理好邻接三角形的边界。
- 使用重心坐标插值公式计算 varying 插值。 
- 使用二次线性插值进行纹理采样，更好的渲染效果。
- 核心渲染实现仅 200 行，突出易读性。
- 写满中文注释，每一处计算都有解释。
- 多个教程例子，从如何画三角形到模型以及光照。

## 编译运行

编译：

```bash
gcc -O2 sample_07_specular.cpp -o sample_07_specular -lstdc++
```

运行：

```bash
./sample_07_specular
```

然后得到一个图片文件 `output.bmp`：

![](https://raw.githubusercontent.com/skywind3000/images/master/p/renderhelp/model_4.jpg)

## 编程接口

### 着色器变量

主要使用一个 ShaderContext 的结构体，用于 VS->PS 之间传参，里面都是一堆各种类型的 varying。

```cpp
// 着色器上下文，由 VS 设置，再由渲染器按像素逐点插值后，供 PS 读取
struct ShaderContext {
    std::map<int, float> varying_float;    // 浮点数 varying 列表
    std::map<int, Vec2f> varying_vec2f;    // 二维矢量 varying 列表
    std::map<int, Vec3f> varying_vec3f;    // 三维矢量 varying 列表
    std::map<int, Vec4f> varying_vec4f;    // 四维矢量 varying 列表
};
```

### 顶点着色器

外层需要提供给渲染器 VS 的函数指针，并在渲染器的 `DrawPrimitive` 函数进行顶点初始化时对三角形的三个顶点依次调用：

```cpp
// 顶点着色器：因为是 C++ 编写，无需传递 attribute，传个 0-2 的顶点序号
// 着色器函数直接在外层根据序号读取响应数据即可，最后需要返回一个坐标 pos
// 各项 varying 设置到 output 里，由渲染器插值后传递给 PS 
typedef std::function<Vec4f(int index, ShaderContext &output)> VertexShader;
```

每次调用时，渲染器会依次将三个顶点的编号 `0`, `1`, `2` 通过 `index` 字段传递给 VS 程序，方便从外部读取顶点数据。

### 像素着色器

渲染器对三角形内每个需要填充的点调用像素着色器：

```cpp
// 像素着色器：输入 ShaderContext，需要返回 Vec4f 类型的颜色
// 三角形内每个点的 input 具体值会根据前面三个顶点的 output 插值得到
typedef std::function<Vec4f(ShaderContext &input)> PixelShader;
```

像素着色程序返回的颜色会被绘制到 Frame Buffer 的对应位置。

### 绘制三角形

调用下面接口可以绘制一个三角形：

```cpp
bool RenderHelp::DrawPrimitive()
```

该函数是渲染器的核心，先依次调用 VS 初始化顶点，获得顶点坐标，然后进行齐次空间裁剪，归一化后得到三角形的屏幕坐标。

然后两层 for 循环迭代屏幕上三角形外接矩形的每个点，判断在三角形范围内以后就调用 VS 程序计算该点具体是什么颜色。

## 完整例子

现在你想写个 D3D 11 的三角形绘制，没有一千行你搞不定，但是这里我们只需要下面几行：

```cpp
#include "RenderHelp.h"

int main(void)
{
    // 初始化渲染器和帧缓存大小
    RenderHelp rh(800, 600);

    const int VARYING_COLOR = 0;    // 定义一个 varying 的 key

    // 顶点数据，由 VS 读取
    struct { Vec4f pos; Vec4f color; } vs_input[3] = {
        { {  0.0,  0.7, 0.90, 1}, {1, 0, 0, 1} },
        { { -0.6, -0.2, 0.01, 1}, {0, 1, 0, 1} },
        { { +0.6, -0.2, 0.01, 1}, {0, 0, 1, 1} },
    };

    // 顶点着色器，初始化 varying 并返回坐标，
    // 参数 index 是渲染器传入的顶点序号，范围 [0, 2] 用于读取顶点数据
    rh.SetVertexShader([&] (int index, ShaderContext& output) -> Vec4f {
            output.varying_vec4f[VARYING_COLOR] = vs_input[index].color;
            return vs_input[index].pos;        // 直接返回坐标
        });

    // 像素着色器，返回颜色
    rh.SetPixelShader([&] (ShaderContext& input) -> Vec4f {
            return input.varying_vec4f[VARYING_COLOR];
        });

    // 渲染并保存
    rh.DrawPrimitive();
    rh.SaveFile("output.bmp");

    return 0;
}
```

运行结果：

![](https://raw.githubusercontent.com/skywind3000/images/master/p/renderhelp/sample_1.jpg)

