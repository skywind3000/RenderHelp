# RenderHelp

:zap: 可编程渲染管线实现，全中文注释，帮助初学者学习渲染原理。

[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE) [![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://GitHub.com/Naereen/StrapDown.js/graphs/commit-activity) [![Join the chat at https://gitter.im/skywind3000/asynctasks.vim](https://badges.gitter.im/skywind3000/asynctasks.vim.svg)](https://gitter.im/skywind3000/RenderHelp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## 特性介绍

- 单个 RenderHelp.h 文件，从画点开始实现可编程渲染管线，无外部依赖。
- 模型标准，计算精确，使用类 Direct3D 接口。
- 包含一套完整的矢量/矩阵库。
- 包含一套位图 Bitmap 库，方便画点、画线、加载纹理、纹理采样等。
- 使用 C++ 编写顶点着色器 (Vertex Shader) 和像素着色器 (Pixel Shader)，方便断点和调试。
- 使用 Edge Equation 精确计算三角形覆盖范围，处理好邻接三角形的边界。
- 使用重心坐标公式计算 varying 插值。 
- 使用 1/w 进行透视矫正，绘制透视正确的纹理。
- 使用二次线性插值进行采样，更好的渲染效果。
- 核心渲染实现仅 200 行，突出易读性。
- 写满中文注释，每一处计算都有解释。
- 多个教程例子，从如何画三角形到模型以及光照。

## 编译运行

随便找个 `sample_` 开头的例子文件直接 gcc 单文件编译即可：

```bash
gcc -O2 sample_07_specular.cpp -o sample_07_specular -lstdc++
```

在 Mac 下好像要加个 `-std=c++17`，我应该没用啥 17 的东西，不过没环境不太确定。某些平台下可能要加一个 `-lm` ，显示声明一下链接数学库。

运行：

```bash
./sample_07_specular
```

然后得到一个图片文件 `output.bmp`：

![](https://raw.githubusercontent.com/skywind3000/images/master/p/renderhelp/model_4_s.jpg)

本项目的模型使用的是 [tinyrender](https://github.com/ssloy/tinyrenderer) 里面的开源模型。

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

现在你想写个 D3D 12 的三角形绘制，没有一千行你搞不定，但是现在我们只需要下面几行：

```cpp
#include "RenderHelp.h"

int main(void)
{
    // 初始化渲染器和帧缓存大小
    RenderHelp rh(800, 600);

    const int VARYING_COLOR = 0;    // 定义一个 varying 的 key

    // 顶点数据，由 VS 读取，如有多个三角形，可每次更新 vs_input 再绘制
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

## 文件列表

| 文件名 | 说明 |
|-|-|
| [RenderHelp.h](RenderHelp.h) | 渲染器的实现文件，使用时 include 它就够了 |
| [Model.h](Model.h) | 加载模型 |
| [sample_01_triangle.cpp](sample_01_triangle.cpp) | 绘制三角形的例子 |
| [sample_02_texture.cpp](sample_02_texture.cpp) | 如何使用纹理，如何设置摄像机矩阵等 |
| [sample_03_box.cpp](sample_03_box.cpp) | 如何绘制一个盒子 |
| [sample_04_gouraud.cpp](sample_04_gouraud.cpp) | 对盒子进行简单高洛德着色 |
| [sample_05_model.cpp](sample_05_model.cpp) | 如何加载和绘制模型 |
| [sample_06_normal.cpp](sample_06_normal.cpp) | 使用法向贴图增强模型细节 |
| [sample_07_specular.cpp](sample_07_specular.cpp) | 绘制高光 |

## 实现对比

十多年前我写了个软渲染器教程 [mini3d](https://github.com/skywind3000/mini3d)，比较清晰的说明了软件渲染器的核心原理，这是标准软渲染器的实现方法，主要是基于 Edge Walking 和扫描线算法。

而本项目的实现方式是仿照 GPU 的 Edge Equation 实现法，以 mini3d 代表的实现方法其实相对比较复杂，但是很快，适合做 CPU 实时渲染。而本项目模拟 GPU 的实现方式相对简单直观，但是计算量很大，不适合 CPU 实时，却适合 GPU 粗暴的并行处理。

网上有很多可编程渲染管线的实现教程，但是很多都做的有问题，诸如屏幕坐标他们取的是像素方格左上角的点，其实应该取像素方格中心的那个点，不然模型动起来三角形边缘会有跳变的感觉；比如临接三角形的边该怎么处理，基本我没见到几个处理正确的；再比如纹理采样时整数坐标换算应该要四舍五入的，不然纹理旋转起来几个顶点位置不够稳定，会有微动的迹象；还有一些软件渲染器连纹理都不是透视正确的，还在用着仿式纹理映射。。。。

渲染器实现有很多非常细节的地方，如果注意不到，其实渲染结果是不准确的，本项目使用标准模型，不错绘一个点，不算错一个坐标。

再一个是易读性，某些项目为了刻意减少代码量，砍了不少细节处理不说，很多运算都是一大堆矩阵套矩阵，连个出处和说明都没有，这对于初学者来讲是十分费解的，你连公式或者概念的名字都不知道，搜都没得搜。

## 阅读说明

本项主文件 `RenderHelp.h` 一共一千多行，三分之一都是中文注释，复杂运算我全部展开了，并不一味为了节省代码尺寸牺牲可读性，某些计算其实可以提取到外层这样性能更快一些，但是为了可读性，我还是写到了和它相关的位置上，这样阅读理解更轻松。

基本原理，我在下面回答里解释过：

- [OpenGL 和 DirectX 是如何在只知道顶点的情况下得出像素位置的？](https://www.zhihu.com/question/48299522/answer/799333394)

阅读时，代码前面基本都是一些工具库，可以从最后 200 行阅读即可，每个公式我都写了出处，基本半个小时拿笔推导下，你不但能理解渲染器的原理是啥，还多了一个方便随时调试 shader 验证想法的工具。

## Credit

代码不理解可以在 issue 里提问，这样该问题经过回答放在那里也对后来的人有帮助，欢迎 PR 增强功能，补充各类高级渲染效果。

