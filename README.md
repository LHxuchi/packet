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