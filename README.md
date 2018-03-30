# idrcom

## 简介

本项目是城市热点客户端在 OpenWrt／Linux／macOS 上的移植。

## 特征

本项目是使用 C++ 编写，依赖于 libpthread 的轻量级认证客户端。它的设计原则为低资源占用，为嵌入式设备提供稳定的认证服务。

- 长期稳定运行
- 低内存占用
- 断线自动重连
- 配置简单

## 安装

### OpenWrt

下载适合路由器平台的安装包，已提供安装包的平台：ramips、ar71xx

   ```bash
   # 安装 pthread 依赖
   opkg install libpthread
   # 安装 idrcom
   opkg install idrcom_1.0.0-1_ramips_24kec.ipk
   ```

其他平台安装可以自行[编译](#编译)

### Linux & macOS

   ```bash
   git clone https://github.com/koho/idrcom.git
   cd idrcom/src
   make
   make install
   ```

## 使用

输入`idrcom -h`可以查看支持的命令

   ```
   Usage: idrcom [options]
   Options:
     -c FILE    Config file to start authentication
                Default is /etc/idrcom.conf
     -b         Run in background
     -d DELAY   Start service after seconds
   ```

举个栗子

   ```bash
   # 使用 /etc/idrcom.conf 配置文件启动认证
   idrcom
   # 使用 ~/idrcom-eth0.conf 进行认证
   idrcom -c ~/idrcom-eth0.conf
   # 在后台进行认证
   idrcom -c ~/idrcom-eth0.conf -b
   # 后台认证且延迟10秒启动
   idrcom -c ~/idrcom-eth0.conf -b -d 10
   ```

## 配置

本项目配置文件的格式为

   ```bash
   # 同时设置用户名和密码
   user name password
   # 分别设置用户名／密码
   user name
   password password
   ```
下面列出所有支持的配置命令。在括号 [] 里面的命令表示可选，有两个参数的命令可以省略最后一个并通过相应名称设置。

   ```bash
   # 用户名和密码
   user name [password]
   # 服务器和端口
   server ip [port]
   # 接口名
   interface ifn
   # 本地端口
   [listen port]
   # 主机名
   [hostname name]
   # DNS
   [dns ip [dnss]]
   # DHCP服务器
   [dhcp ip]
   ```

配置文件只是向服务器提交的信息，并不需要与系统设置相符，也不会修改系统设置。

## 编译

下载目标平台的 [OpenWrt SDK](https://downloads.openwrt.org/)，配置[编译环境](https://openwrt.org/docs/guide-developer/obtain.firmware.sdk)

   ```bash
   # 以 ramips 平台为例
   tar xjf OpenWrt-SDK-15.05.1-ramips-rt305x_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64.tar.bz2
   cd OpenWrt-SDK-15.05.1-ramips*
   # 获取 idrcom
   git clone https://github.com/koho/idrcom.git package/idrcom
   # 开始编译
   make package/idrcom/compile V=99
   # 复制到路由器
   scp bin/ramips/packages/base/idrcom_1.0.0-1_ramips_24kec.ipk root@192.168.1.1:/tmp
   ```
