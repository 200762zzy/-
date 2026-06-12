# 实验报告AI自动评语生成工具

基于 **Qt 5.14.2** + **DeepSeek API** 的 Windows 桌面工具，自动从 `.docx` 实验报告中提取实验目的/名称/内容/成绩，一键批量生成 AI 教师评语。

## 功能特性

- **自动提取实验信息** — 从 docx 中读取实验名称、实验目的、实验内容，无需手动输入
- **批量处理** — 一次性处理整个文件夹的所有 `.docx` 文件
- **AI 评语生成** — 调用 DeepSeek 大模型，根据实验内容和成绩生成针对性评语
- **成绩自动识别** — 自动从文档中提取分数（支持多关键词匹配）
- **备份机制** — 可选在子文件夹保存副本，保留原始文件
- **高度自定义** — 可配置 API 地址、模型参数、温度等

## 使用方法

1. **下载** — 从 [GitHub Releases](https://github.com/200762zzy/-/releases) 下载最新版 `classObject2_v*.zip`
2. **解压** — 解压到任意目录，运行 `classObject2.exe`
3. **配置 API** — 输入 DeepSeek API 地址和 Key
4. **选择文件夹** — 选择存放学生实验报告 `.docx` 的文件夹
5. **开始处理** — 点击"开始处理"，程序自动提取成绩和实验信息 → 生成评语 → 写入文档

> 提示词为可选字段。留空时，AI 将仅根据文档中已有的实验目的/名称/内容和成绩生成评语。

## 运行环境

- Windows 7 及以上
- 无需安装 Qt 运行时（已打包所有依赖 DLL）
- 需要有效的 [DeepSeek API Key](https://platform.deepseek.com/)

## 构建说明

```bash
# 环境：Qt 5.14.2 MinGW 32-bit
qmake classObject2.pro
mingw32-make -f Makefile.Release
windeployqt release\classObject2.exe
```

## 项目结构

```
classObject2/
├── main.cpp            # 入口
├── mainwindow.h/cpp    # 主界面 UI + 交互逻辑
├── deepseekapi.h/cpp   # DeepSeek API 封装（同步调用）
├── docxhelper.h/cpp    # docx 文件的解压/解析/写入
├── workerthread.h/cpp  # 后台处理线程
├── classObject2.pro    # Qt 项目文件
└── release/            # 构建输出（已打包所有依赖）
```

## License

MIT
