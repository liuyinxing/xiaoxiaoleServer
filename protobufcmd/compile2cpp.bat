    rem 切换到.proto协议所在的目录
    cd  proto\hall
    rem 将当前文件夹中的所有协议文件转换为lua文件
    for %%i in (*.proto) do (  
    echo %%i
    "..\..\protoc.exe"  --cpp_out=..\..\out %%i
    )
    echo end
	
	cd  ..\games
    rem 将当前文件夹中的所有协议文件转换为lua文件
    for %%i in (*.proto) do (  
    echo %%i
    "..\..\protoc.exe"  --cpp_out=..\..\out %%i
    )
    echo end
	
    pause