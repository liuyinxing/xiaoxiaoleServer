protoc.exe --cpp_out=.\   .\Account.proto
move Account.pb.h   ..\share\SDK\protobuf
move Account.pb.cc .\protobuf2lib\libprotobufd\protobuflib
protoc.exe --cpp_out=.\   .\sendMd5.proto
move sendMd5.pb.h   ..\share\SDK\protobuf
move sendMd5.pb.cc .\protobuf2lib\libprotobufd\protobuflib
protoc.exe --cpp_out=.\   .\socket.proto
move socket.pb.h   ..\share\SDK\protobuf
move socket.pb.cc .\protobuf2lib\libprotobufd\protobuflib
pause
