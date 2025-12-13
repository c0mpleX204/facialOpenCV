# facialOpenCV 项目依赖说明（Windows / C++）

本项目依赖 OpenCV（推荐 4.10 及更高版本）。下面提供两种可行的安装与集成方式：优先推荐 vcpkg，其次是从 GitHub 源码自行构建并安装。

## 方式一：使用 vcpkg 安装（推荐）

1. 安装 vcpkg（如未安装）：

	```powershell
	# 建议统一安装到 C:\tools\vcpkg，便于多项目复用
	git clone https://github.com/microsoft/vcpkg C:\tools\vcpkg
	& "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File C:\tools\vcpkg\scripts\bootstrap.ps1
	```

2. 安装 OpenCV 4.10+（根据需要选择是否包含 contrib 模块）：

	- 仅基础模块：
	  ```powershell
	  C:\tools\vcpkg\vcpkg.exe install opencv:x64-windows --recurse
	  ```
	- 含 contrib 模块：
	  ```powershell
	  C:\tools\vcpkg\vcpkg.exe install opencv[contrib]:x64-windows --recurse
	  ```

	如需固定版本，可以使用清单模式（`vcpkg.json`）锁定版本，或使用 `x-update` 后执行 `vcpkg` 的版本解析。默认安装的是 vcpkg 可用的最新稳定版本，满足“推荐 4.10+”。

3. CMake 集成（最小示例）：

	在项目的 `CMakeLists.txt` 中（示例）：

	```cmake
	cmake_minimum_required(VERSION 3.20)
	project(facialOpenCV CXX)

	# 指定 vcpkg 工具链（也可在命令行传入）
	set(VCPKG_ROOT "C:/path/to/vcpkg")
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

	find_package(OpenCV REQUIRED)

	add_executable(facialOpenCV src/main.cpp)
	target_link_libraries(facialOpenCV PRIVATE ${OpenCV_LIBS})
	target_include_directories(facialOpenCV PRIVATE ${OpenCV_INCLUDE_DIRS})
	```

	构建命令：

	```powershell
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="C:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
	cmake --build build --config Release
	```

4. 运行时依赖（DLL）：

	- 如果运行时提示缺少 DLL，可将 `C:\path\to\vcpkg\installed\x64-windows\bin` 加入系统或用户 `PATH`，或将所需 DLL 复制到可执行文件输出目录（例如 `build/Release`）。

> 说明：vcpkg 会把 OpenCV 安装在 `C:/tools/vcpkg/installed/x64-windows/` 下，并通过工具链文件让 CMake 自动找到头文件、库与配置；无需将 OpenCV 源码放入项目根目录。

## 方式二：从 GitHub 源码构建并安装

适用于需要自定义编译选项（如启用 `opencv_contrib` 模块、CUDA 加速、或修改源码）。流程如下：

1. 克隆源码：

	```powershell
	git clone https://github.com/opencv/opencv.git
	git clone https://github.com/opencv/opencv_contrib.git
	```

2. 配置与生成（out-of-source build 推荐）：

	```powershell
	mkdir opencv-build
	cd opencv-build

	cmake -G "Visual Studio 17 2022" ^
	  -D CMAKE_BUILD_TYPE=Release ^
	  -D OPENCV_EXTRA_MODULES_PATH="../opencv_contrib/modules" ^
	  -D BUILD_EXAMPLES=OFF ^
	  -D BUILD_TESTS=OFF ^
	  ../opencv
	```

3. 编译与安装：

	```powershell
	cmake --build . --config Release -- /m
	cmake --install . --config Release --prefix "C:\opencv-install"
	```

4. 在你的项目中集成：

	在 `CMakeLists.txt` 中指向安装前缀的 cmake 配置路径（不同生成器路径略有差异）：

	```cmake
	set(OpenCV_DIR "C:/opencv-install/lib/cmake/opencv4")
	find_package(OpenCV REQUIRED)

	add_executable(facialOpenCV src/main.cpp)
	target_link_libraries(facialOpenCV PRIVATE ${OpenCV_LIBS})
	target_include_directories(facialOpenCV PRIVATE ${OpenCV_INCLUDE_DIRS})
	```

	运行时同样需要把 `C:\opencv-install\bin` 加入 `PATH` 或复制 DLL 到输出目录。

> 说明：不要将 OpenCV 源码直接放入项目根目录进行编译/链接；应当链接其安装产物。源码构建给你最大灵活性，但 vcpkg 更简单。

## 版本与仓库说明

- GitHub 仓库 `opencv/opencv` 提供源码与构建脚本；发行版本（Releases）中可查看具体版本标签。
- vcpkg 通常提供稳定的预编译/可构建的配方，安装的版本满足“推荐 4.10+”。如需精准版本，使用 vcpkg 清单模式或从源码按指定标签构建。
- `opencv_contrib` 仓库包含额外模块（如特征、ximgproc 等），需通过 `OPENCV_EXTRA_MODULES_PATH` 在构建时加入。

## 小结

- vcpkg：最快落地，自动集成，适合多数 C++ 项目。
- 源码构建：灵活可定制，适合需要特定编译选项的场景。
- 两种方式都不需要把 OpenCV 源码放到项目根目录；正确方式是在 CMake 中 `find_package(OpenCV)` 并链接其安装库，同时处理运行时 DLL。

## 使用 OpenCV Zoo 的 YuNet 进行人脸检测（推荐方案）

为满足“不要使用 `CascadeClassifier`，推荐使用 OpenCV Zoo 的 YuNet 模型”的要求，本项目建议使用 YuNet（`face_detection_yunet.onnx`）进行人脸检测，再对检测到的人脸区域进行模糊或遮挡。

### 前置条件
- OpenCV 版本：4.10 或更高（`DNN` 模块与 `FaceDetectorYN` 接口在新版支持更好）。
- 通过 vcpkg 安装 OpenCV（基础包即可，不强制 `contrib`）：
	```powershell
	C:\tools\vcpkg\vcpkg.exe install opencv:x64-windows --recurse
	```
	若你已安装 `opencv[contrib]` 也可使用，功能不受影响。

### 获取 YuNet 模型文件
- 访问 OpenCV Zoo YuNet 模型页面：
	- 说明与下载（官方仓库）：https://github.com/opencv/opencv_zoo/tree/main/models/face_detection_yunet
	- 直接模型文件：`face_detection_yunet.onnx`
- 将模型文件保存到项目目录下的 `models/face_detection_yunet.onnx`。

### 开发流程概述（图片人脸模糊/遮挡）
1. 加载输入图片（如 `test.jpg`）。
2. 初始化 YuNet 检测器：
	 - 使用 OpenCV 提供的 `FaceDetectorYN`（推荐）或通过 `cv::dnn::readNet` 直接加载 ONNX，并设置输入尺寸/置信度阈值。
	 - 常见参数含：输入宽高（与图片或缩放后的尺寸一致）、置信度阈值、NMS 阈值、选用后端/目标（如 `DNN_BACKEND_OPENCV`、`DNN_TARGET_CPU`）。
3. 进行人脸检测，获得人脸框（`Rect`/`cv::Rect` 列表）。
4. 对每个框执行你选择的保护方式：
	 - 模糊：`GaussianBlur`（在 ROI 上应用较大核）。
	 - 像素化：先缩小 ROI 再用 `INTER_NEAREST` 放大回去。
	 - 遮挡：在 ROI 上绘制实心矩形（黑框或自定义样式）。
5. 输出并保存处理结果图片（如 `out.jpg`）。

### 代码结构建议（不在项目内自动生成代码，仅作纲要）
- 初始化与参数：
	- 程序入口解析：`<input> <mode> <output>`。
	- 模型路径：`models/face_detection_yunet.onnx`。
- 检测器创建（示例思路）：
	- `FaceDetectorYN::create(modelPath, config, inputSize, scoreThresh, nmsThresh, topK)`；或使用 `cv::dnn` 手动前向推理并解析输出。
- 推理流程：
	- 将图片转换为所需输入尺寸（必要时缩放）；
	- 执行 `detect()` 获取人脸框；
- 处理与保存：
	- 根据 `mode` 对 ROI 执行模糊/像素化/遮挡；
	- 写出目标路径。

### 运行与调试要点
- DLL 路径：如运行时报缺少 OpenCV DLL，请将 `C:\path\to\vcpkg\installed\x64-windows\bin` 加入 `PATH` 或复制到可执行输出目录。
- 性能与精度：YuNet 比 Haar 更鲁棒；如需更高精度/速度，可设置合适的输入尺寸与阈值，或考虑 GPU 加速（依据你的 OpenCV/DNN 构建选项）。
- 模型版本：保持模型与 OpenCV 版本兼容；如遇到接口差异，请参考 YuNet 官方 README 的 API 示例与参数说明。

### 我能为你做什么（按需）
- 如果你需要，我可以：
	- 提供最小可运行的 C++ 代码片段（不写入项目文件，先供你确认）。
	- 在你确认后，将示例代码添加到指定路径（例如 `src/`）并配置 `CMakeLists.txt`，以便一键构建运行。
	- 或改为基于 `cv::dnn` 的直接 ONNX 推理示例，便于自定义后端或移植。


