#include <uvw.hpp>
#include <iostream>

std::string socket_path("\0echo.sock", sizeof("\0echo.sock")-1);

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
	







	pipe->connect(socket_path);



    //pipe->on<uvw::DataEvent>([](auto& evt, auto& hndl){
	//		std::cout<<"Got something: "<<std::endl;
	//		// The event argument is a struct with only two members: A unique_ptr
	//		// to a char array and an integer with the length.
	//		std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
    //});
	//pipe->read();




	// Create a resource that will listen to STDIN
    auto console = loop->resource<uvw::TTYHandle>(uvw::StdIN, true);
    console->on<uvw::DataEvent>([pipe](auto& evt, auto& hndl){
			(void) hndl;
			std::cout<<"Got something: "<<std::endl;
			// The event argument is a struct with only two members: A unique_ptr
			// to a char array and an integer with the length.
			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
			//pipe->write(evt.data, evt.length);
			pipe->write(&evt.data[0], evt.length);
    });
	console->read();


    loop->run();
}
