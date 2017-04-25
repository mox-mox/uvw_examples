#include <uvw.hpp>
#include <iostream>

//std::string socket_path("\0echo.sock", sizeof("\0echo.sock")-1);
std::string socket_path("echo.sock", sizeof("echo.sock")-1);

int main()
{
	// Create a loop
    auto loop = uvw::Loop::getDefault();



	// Create a resource that will listen to STDIN
    auto pipe = loop->resource<uvw::PipeHandle>();
	pipe->init();
    pipe->on<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::PipeHandle &) {
        //handle.close();
			std::cout<<"Wrote to the pipe"<<std::endl;
    });

    pipe->on<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        handle.tryWrite(std::move(dataTryWrite), 1);
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        handle.write(std::move(dataWrite), 2);
    });
	
    pipe->on<uvw::DataEvent>([](const uvw::DataEvent& evt, auto&){
			std::cout<<"Got something from the pipe: "<<std::endl;
			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
    });

	pipe->connect(socket_path);
	pipe->read();




	// Create a resource that will listen to STDIN
    auto console = loop->resource<uvw::TTYHandle>(uvw::StdIN, true);
    console->on<uvw::DataEvent>([pipe](auto& evt, auto& hndl){
			(void) hndl;
			std::cout<<"Got something from STDIN: "<<std::endl;
			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
			pipe->write(&evt.data[0], evt.length);
    });
	console->read();


    loop->run();
}
