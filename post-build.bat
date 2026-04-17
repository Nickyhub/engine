cd 

echo "assets\shaders\MaterialShader.vert.glsl -> assets\shaders\MaterialShader.vert.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=vert assets\shaders\MaterialShader.vert.glsl -o assets\shaders\MaterialShader.vert.spv
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "assets\shaders\MaterialShader.frag.glsl -> assets\shaders\MaterialShader.frag.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=frag assets\shaders\MaterialShader.frag.glsl -o assets\shaders\MaterialShader.frag.spv
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)


xcopy "assets" "build/assets" /h /i /c /k /e /r /y
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
popd
echo "Done."
PAUSE