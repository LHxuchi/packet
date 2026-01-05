# packet
## 依赖安装
```bash
sudo apt-get update

sudo apt-get install libssl-dev

sudo apt-get install build-essential
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
sudo apt-get install qtcreator
sudo apt-get install qt5*

sudo apt-get install libcatch2-dev
```
若出现无法定位软件包 libcatch2-dev，则再运行如下命令安装Catch2
```bash
git clone https://github.com/catchorg/Catch2.git
cd Catch2
mkdir build
cd build
cmake..
make
sudo make install
```

## 项目构建
下载该项目源代码后，到项目目录运行如下指令构建
```bash
mkdir build
cd build
cmake ..
make
```
## 项目运行
当项目构建成功后，相关的可执行程序会生成到bin目录下
若要执行单元测试，则执行如下命令
```bash
cd bin
./utest
```
若要执行图形化程序，则执行如下命令
```bash
cd bin
./backup_gui
```