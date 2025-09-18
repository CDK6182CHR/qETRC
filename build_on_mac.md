本文件用来说明如何将qETRC程序以.dmg以及.app形式安装保存在mac电脑上，全程在终端内操作

一、安装开发环境和依赖工具
    开发环境：
        安装Xcode命令行工具：xcode-select --install
        确认安装：clang --version
        安装homebrew：/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    依赖工具：
        安装Cmake、Git:brew install cmake git
        确认版本：cmake --version（version>=3.14）
                git --version
        安装Qt6（version>=6.5.1）:
            1、前往官网安装（推荐--全面）
            2、homebrew安装命令：brew install qt6

二、克隆源代码并同步子模块SARibbon
    新建一个专门用来存放代码的文件夹（推荐--干净、方便管理）
    在终端中进入该工作目录，执行命令：
    git clone https://github.com/CDK6182CHR/qETRC.git
    cd qETRC
    git submodule update --init --recursive

三、设置Qt环境变量
    找到Qt安装目录，例如：/opt/Qt/6.5.1/clang_64
    执行(文件位置按照自己保存的位置设置，如果安装官方文件，安装位置一般如下，除“/Users/yqyqyq”这一段外其他不要变,但注意6.9.2这个文件夹可能随版本更新而改变)：
    export Qt6_DIR=/Users/用户名/Qt/6.9.2/macos/lib/cmake/Qt6
    export PATH=/Users/用户名/Qt/6.9.2/macos/bin:$PATH
    如果要重复安装程序，必须每次都指定好环境变量

四、执行build_mac.sh
    build_mac.sh是一个自动化部署整个程序的脚本文件
    在终端qETRC路径下执行：chmod +x build_mac.sh 给予文件可执行权限
    执行：./build_mac.sh

五、完成程序部署

六、可能遇见的问题及其解决办法
    1、运行中断提示没有某一个或多个库函数：
    电脑中安装了多个版本的Qt,且存在不适用的版本--删除多的且不适用于该程序版本的Qt
    2、app可以安装好但是未能打开：
    可能是脚本文件中中有关Qt的位置不对，可以尝试将Qt的位置添加到环境变量中（即步骤三），或查看修改”步骤三“的路径
    3、程序执行完成后会有一些报错“285:347: execution error: “Finder”遇到一个错误：不能将“icon view options of container window of disk "qETRC"”设置为“icon view options”。 (-10006)”，无伤大雅，不用理会
    4、有关图标问题可能导致一些报错，可能的原因是访达没有相应权限：
    在设置中，找到隐私与安全性，找到完全磁盘访问权限，进入后打开访达的选项，重新进行程序部署

