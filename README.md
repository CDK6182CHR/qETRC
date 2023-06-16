# qETRC

Electronic Train Running Chart implemented with Qt

> To build this project from source code, see [build](doc/build.md)

本项目是对此前的[pyETRC](https://github.com/CDK6182CHR/train_graph)项目的C++重构版本，工程名暂定为qETRC。本项目基于GPLv3协议开放源代码，在协议允许范围内，作者保留一切权利和最终解释权。

联系方式：mxy0268@qq.com

在线文档（尚未完成）：https://qetrc.readthedocs.io/



## 环境

自2023年6月16日起，改为使用CMake构建，原基于qmake的构建流程作废。详见[build](doc/build.md)



## 与pyETRC的关系

本项目定位为采用C++重构版本的pyETRC，并做一定的扩展。进行这个重构是因为Python的动态类型难于拿捏，在过去三年pyETRC的开发过程中，由于数据结构设计的草率等原因，代码难以维护。另外，使用Python开发时，对对象生命周期的控制较弱（由于赋值语义问题，多个对象共享同一个数据对象时容易出问题），对数据结构的控制不够精细（并不是说不能，但用Python写链表之类的东西总是怪怪的），这些限制了原有程序的重构。

qETRC重新设计了数据结构，新增**线路区间**（`RailInterval`）的抽象，并采用类似双链表的形式组织数据，使得基于区间的数据（标尺、天窗）等有了实际的依托，理论上更加安全。此外，依托新的界面，对原有框架进行扩展，由仅支持一条线路、一个运行图窗口拓展到支持多条线路和多个运行图窗口同时打开。

qETRC远期计划完全替代pyETRC，实现绝大多数的pyETRC既有功能，除了一些不大常用或容易引起问题的小功能。目前比较明确将会丢弃的功能是手动运行线管理功能。

## 基本概念

本项目数据的最高级别抽象（i.e. 最大范围）为**运行图**(`Diagram`)，每个程序实例在同一时刻仅处理一个运行图对象。每个运行图对象含有以下基础数据：

- 一组**基线**数据 （`Railway`）
- 一组**列车**数据 （`Train`）

除此之外，每个运行图可以具有若干个**视窗**/**页面** （`DiagramPage`），它与程序图形界面中的运行图窗口（`DiagramWidget`）是一一对应的。每个运行图页面可以包含一条或者多条基线。

在设计数据结构和算法时，原则上，考虑以下数据规模：

- 列车数量约在**1000量级**。

- 每条基线的车站数量约在**100量级**。

  > 以上两项数据规模与pyETRC通常能处理的数据规模类似。在此框架下，程序中将尽量避免对这两项数据进行线性查找，回避平方以上复杂度的算法。

- 基线数量约在**10量级**。

- 运行图视窗约在**10量级**。

  > 以上两项为初期设计原则，即项目初期仅仅考虑少量线路集合的运行图文件。对于线路和运行图视窗，将不介意采用线性查找。



## 项目结构

以下为开发者提供一些初步的信息。关于具体的文件、类、函数的结构及其说明，可以采用Doxygen等工具生成文档浏览。以下给出基本的文件结构（至文件夹层次）及其划分逻辑：

- `data` 数据部分。运行图、基线、列车等数据结构的实现。
  - `common` 比较简单的公共数据结构，例如站名`StationName`
  
  - `rail` 铁路线路数据。总成在`Railway`类中，包含以下几个部分：
    
    - `RailStation` 一个车站，包括站名、延长公里等。
    - `RailInterval` 两个车站的一个区间。这是相比于pyETRC新增的数据结构。
    - 基于线路区间的数据，包括标尺`Ruler`、天窗`Forbid`。
    
  - `train` 列车数据结构。主要包括
    - `Train` 列车，包含车次、时刻表以及始发终到站等数据。
    - `Routing` 交路。是一组列车的序列。
    - `TrainCollection` 列车集合。包含一组列车、一组交路和一套列车类型系统。
    
  - `diagram` 运行图级别数据结构。主要包括运行图`Diagram`和运行图页面`DiagramPage`等。
  
  - `calculation` 此部分主要与贪心推线算法有关。
  
  - `analysis` 运行图分析相关算法。
  
    - `inttrains` 区间对数表/车次表
    - `traingap` 列车间隔计算相关。
  
    注意由于历史原因，大量相关的算法（例如列车间隔计算的核心代码，标尺综合等）仍在`Diagram`或`TrainGap`等类中。原则上应当留待重构。
  
  - `gapset` 列车间隔分组相关。
    注意由于历史原因，`traingap.h/.cpp`文件目前在`diagram`目录下，但逻辑上认为它应属于本文件夹。
  
- `dialogs` 一些基于对话框实现的较为简单的功能，例如导入车次`ImportTrainDialog`。

- `editors` 原pyETRC中采用停靠面板实现的、核心数据结构的编辑控件，例如线路里程编辑、车次编辑等。

- `kernel` 绘图核心模块，实现运行图绘制。

- `mainwindow` 程序主窗口以及相关的界面逻辑类。
  注意本目录下的所有文件及少量其他类在移动版中不编译。

- `model` 基于Qt的Model/View框架设计的数据模型。其中
  - `delegate` 编辑的代理类。
  - `train` 与列车相关的模型，例如时刻表。
  - `rail` 与线路相关的模型，例如线路里程表。
  - `diagram` 超过以上两个抽象层次的模型。
  
- `mobile` 专为移动版设计的界面类，目前主要是主界面以及对相关功能的调用。
  
- `navi` 是qETRC新增部分，与导航窗口相关的界面程序。

- `railnet` 路网管理部分，包含原pyETRC中的线路数据库以及（外置）路网管理模块的对应实现。

  - `raildb` 线路数据库。包含数据和界面。
  - `graph` 线网有向图模型，包括数据实现和有向图模型浏览器等。
  - `path` 路径选择算法、快速线路生成功能实现。

- `viewers` 原pyETRC中，运行图分析的功能，例如事件表等。

- `railnet` 中远期规划，原pyETRC线网管理模块的功能。

- `util` 一些杂项，对Qt的轻度扩展等。

- `wizards` 向导，或按照类似向导逻辑实现的功能。

  - `readruler` 标尺综合。
  - `selectpath` 经由选择器。
  - `timeinterp` 时刻插值算法。
  - `rulerpaint` 标尺排图向导。
  - `greedypaint` 贪心推线向导。




## 致谢

本项目使用了以下的开源库：

- [Advanced-Docking-System](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System)：提供了任意组织停靠面板的功能。
- [SARibbon](https://github.com/czyt1988/SARibbon): 提供了Ribbon风格工具栏的功能。

衷心感谢以上项目相关开发者的工作及其提供的便利。



## 捐赠

qETRC目前免费开源。

如果本项目对你有帮助，或你喜欢本项目，可以捐赠开发者。

感谢支持！

<div>
    <div>
        <img src="img/QRcode.jpg" width="200px"/>
    </div>
    <div>
        <img src="img/myoisinh.png" width="200px" />
    </div>
</div>


