此目录用于存放人脸检测模型（如 YuNet 的 ONNX 文件）。

建议：在项目根运行 `scripts\download_yunet.ps1` 自动下载模型：

PowerShell（在项目根下运行）：
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\download_yunet.ps1
```

下载后程序会从 `models/face_detection_yunet_2023mar.onnx` 加载模型。
