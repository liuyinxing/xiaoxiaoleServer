    rem 切换到.proto协议所在的目录
    cd  out
    rem 将当前文件夹中的所有协议文件转换为lua文件
    for %%i in (*.h) do (  
    echo %%i
		copy %%i ..\..\..\SDK\protobuf\%%i
    )
    echo end
	
    pause