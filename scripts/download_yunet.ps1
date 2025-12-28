Param()

# Downloads YuNet ONNX model into the project's models/ directory.
$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
$proj = Resolve-Path "$root\.."
$modelsDir = Join-Path $proj "models"
if (-not (Test-Path $modelsDir)) { New-Item -ItemType Directory -Path $modelsDir | Out-Null }

$url = 'https://raw.githubusercontent.com/opencv/opencv_zoo/master/models/face_detection_yunet/face_detection_yunet_2023mar.onnx'
$out = Join-Path $modelsDir 'face_detection_yunet_2023mar.onnx'

Write-Host "Downloading YuNet model from:`n $url`n to:`n $out`n"
try {
    Invoke-WebRequest -Uri $url -OutFile $out -UseBasicParsing -ErrorAction Stop
    Write-Host "Download complete." -ForegroundColor Green
} catch {
    Write-Host "Download failed: $_" -ForegroundColor Red
    Write-Host "If the URL is blocked, please download manually from OpenCV Zoo and place the ONNX file at: $out"
    exit 1
}
