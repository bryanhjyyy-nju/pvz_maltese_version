param(
    [string]$QtRoot = 'C:\Qt\5.15.2\mingw81_64',
    [string]$CompilerRoot = 'C:\Qt\Tools\mingw810_64',
    [switch]$Test,
    [switch]$Deploy,
    [switch]$BuildOnly
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildDirectory = Join-Path $projectRoot $(if ($Test) { 'build-qt5-tests' } else { 'build-release' })
$qmake = Join-Path $QtRoot 'bin\qmake.exe'
$make = Join-Path $CompilerRoot 'bin\mingw32-make.exe'
if (!(Test-Path $qmake) -or !(Test-Path $make)) {
    throw '找不到 Qt / MinGW。请用 -QtRoot 和 -CompilerRoot 指定匹配的安装目录。'
}
$env:PATH = "$QtRoot\bin;$CompilerRoot\bin;$env:PATH"
New-Item -ItemType Directory -Force $buildDirectory | Out-Null
Push-Location $buildDirectory
try {
    $projectFile = Join-Path $projectRoot $(if ($Test) { 'pvz_tests.pro' } else { 'PvZ_demo.pro' })
    & $qmake $projectFile 'CONFIG+=release' 'CONFIG-=debug'
    if ($LASTEXITCODE -ne 0) { throw 'qmake 失败；请确认 Qt Multimedia 模块已安装。' }
    & $make '-j4'
    if ($LASTEXITCODE -ne 0) { throw '编译失败。' }
    if ($Test -and !$BuildOnly) {
        $env:QT_QPA_FONTDIR = Join-Path $env:WINDIR 'Fonts'
        $env:PVZ_CAPTURE_DIR = Join-Path $buildDirectory 'screenshots'
        & '.\release\pvz_tests.exe' '-platform' 'offscreen' '-o' 'test-results.txt,txt'
        $testExitCode = $LASTEXITCODE
        Get-Content 'test-results.txt'
        if ($testExitCode -ne 0) { throw '测试失败。' }
    } elseif ($Deploy) {
        $output = Join-Path $buildDirectory 'dist'
        New-Item -ItemType Directory -Force $output | Out-Null
        Copy-Item -LiteralPath (Join-Path $buildDirectory 'release\PvZ_demo.exe') -Destination $output -Force
        $pluginFolders = @('platforms','audio','mediaservice','imageformats')
        foreach ($name in @('Qt5Core.dll','Qt5Gui.dll','Qt5Multimedia.dll','Qt5Network.dll','Qt5Widgets.dll',
                            'libgcc_s_seh-1.dll','libstdc++-6.dll','libwinpthread-1.dll')) {
            $source = if ($name.StartsWith('Qt5')) { Join-Path $QtRoot "bin\$name" } else { Join-Path $CompilerRoot "bin\$name" }
            if (!(Test-Path $source)) { throw "缺少运行库：$source" }
            Copy-Item -LiteralPath $source -Destination $output -Force
        }
        $plugins = @(
            @{ Source = 'platforms\qwindows.dll'; Destination = 'platforms' },
            @{ Source = 'audio\qtaudio_windows.dll'; Destination = 'audio' },
            @{ Source = 'mediaservice\qtmedia_audioengine.dll'; Destination = 'mediaservice' },
            @{ Source = 'imageformats\qgif.dll'; Destination = 'imageformats' },
            @{ Source = 'imageformats\qjpeg.dll'; Destination = 'imageformats' }
        )
        foreach ($plugin in $plugins) {
            $source = Join-Path $QtRoot "plugins\$($plugin.Source)"
            if (!(Test-Path $source)) { throw "缺少 Qt 插件：$source" }
            $destination = Join-Path $output $plugin.Destination
            New-Item -ItemType Directory -Force $destination | Out-Null
            Copy-Item -LiteralPath $source -Destination $destination -Force
        }
        Write-Output "可运行目录：$output"
    }
} finally {
    Pop-Location
}
