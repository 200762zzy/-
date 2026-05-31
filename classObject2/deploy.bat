@echo off
chcp 65001 >nul
echo ========================================
echo  部署实验报告AI自动评语生成工具
echo ========================================
echo.

setlocal enabledelayedexpansion

:: ==============================================
:: 配置路径 — 根据你的 Qt 安装位置修改
:: ==============================================
set QT_DIR=D:\Qt5.14\5.14.2\mingw73_32
set MINGW_DIR=D:\Qt5.14\Tools\mingw730_32\opt
set BUILD_DIR=D:\qt\classObject2\release
set DEPLOY_DIR=D:\qt\classObject2\deploy_output

:: ==============================================
:: 如果 Qt Creator 编译输出路径不同，修改下面路径
:: 32位 Debug 输出目录：
set BUILD_DIR_QTCR=D:\qt\build-classObject2-Desktop_Qt_5_14_2_MinGW_32_bit-Debug\debug
:: ==============================================

if not exist "%BUILD_DIR%\classObject2.exe" (
    if exist "%BUILD_DIR_QTCR%\classObject2.exe" (
        set BUILD_DIR=%BUILD_DIR_QTCR%
    ) else (
        echo [错误] 找不到 classObject2.exe
        echo 请先在 Qt Creator 中编译项目。
        exit /b 1
    )
)

echo [1/3] 创建部署目录...
if not exist "%DEPLOY_DIR%" mkdir "%DEPLOY_DIR%"

:: 先复制可执行文件和 OpenSSL DLL（windeployqt 可能遗漏它们）
echo [2/3] 复制 OpenSSL DLL（HTTPS 必需）...
if exist "%MINGW_DIR%\bin\libeay32.dll" (
    copy /Y "%MINGW_DIR%\bin\libeay32.dll" "%DEPLOY_DIR%" >nul
    echo   ✓ libeay32.dll
) else (
    echo   [警告] libeay32.dll 未找到，HTTPS 将无法使用
)
if exist "%MINGW_DIR%\bin\ssleay32.dll" (
    copy /Y "%MINGW_DIR%\bin\ssleay32.dll" "%DEPLOY_DIR%" >nul
    echo   ✓ ssleay32.dll
) else (
    echo   [警告] ssleay32.dll 未找到，HTTPS 将无法使用
)

echo [3/3] 运行 windeployqt 收集 Qt 依赖...
"%QT_DIR%\bin\windeployqt.exe" "%BUILD_DIR%\classObject2.exe" --dir "%DEPLOY_DIR%"
if errorlevel 1 (
    echo [警告] windeployqt 完成（可能有非致命警告）
)

:: 确保可执行文件在部署目录中
copy /Y "%BUILD_DIR%\classObject2.exe" "%DEPLOY_DIR%" >nul
echo   ✓ classObject2.exe

echo.
echo ========================================
echo  部署完成！输出目录：%DEPLOY_DIR%
echo ========================================
echo.
echo  运行：%DEPLOY_DIR%\classObject2.exe
echo.
echo  [调试] 如果还连不上，把以下路径的 DLL 也复制过去：
echo  %MINGW_DIR%\bin\libeay32.dll
echo  %MINGW_DIR%\bin\ssleay32.dll
echo.
pause
