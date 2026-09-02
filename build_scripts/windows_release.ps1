$ErrorActionPreference = "Stop"

$PRESET = "windows_release"
$BUILD_DIR = "out/build/$PRESET"
$INSTALL_DIR = "out/install/$PRESET"

Write-Host "Configuring..."
cmake --preset $PRESET
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building..."
cmake --build $BUILD_DIR --config Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Installing..."
cmake --install $BUILD_DIR --config Release --prefix $INSTALL_DIR
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }