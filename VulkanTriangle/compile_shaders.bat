@echo off
echo Compiling shaders...

:: Check if glslc is in PATH, use it directly if available
where glslc > nul 2>&1
if %ERRORLEVEL% == 0 (
    set GLSLC=glslc
) else (
    :: Try to find glslc in Vulkan SDK
    if defined VULKAN_SDK (
        set GLSLC="%VULKAN_SDK%\Bin\glslc.exe"
    ) else (
        echo Error: glslc not found in PATH and VULKAN_SDK environment variable is not set.
        echo Please install the Vulkan SDK and set the VULKAN_SDK environment variable.
        exit /b 1
    )
)

:: Create output directory if it doesn't exist
if not exist "Resources\Shaders\spirv" mkdir "Resources\Shaders\spirv"

:: Compile shaders
%GLSLC% -o Resources\Shaders\spirv\Triangle.vert.spv Resources\Shaders\Triangle.vert
%GLSLC% -o Resources\Shaders\spirv\Triangle.frag.spv Resources\Shaders\Triangle.frag

echo Shader compilation complete!
