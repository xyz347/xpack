
## glog 安装

- 从 github 上下载 glog 项目
  ```shell
    git clone git@github.com:google/glog.git
  ```

- 选择编译分支
  ```shell
    git checkout v0.4.0
  ```
  这里选择 tag 为 `v0.4.0` 的版本进行编译

- 采用 cmake 编译安装方式，指定生成动态库
  ```shell
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local/glog -DBUILD_SHARED_LIBS=ON ..
    make
    make install
  ```

## 效率测试

2-second

snprintf 要比 std::to_string 节省时间 7.4%