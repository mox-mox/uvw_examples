#include <uvw.hpp>
#include <iostream>

//std::string socket_path("\0echo.sock", sizeof("\0echo.sock")-1);
std::string socket_path("echo.sock", sizeof("echo.sock")-1);

int main()
{
	// Create a loop
    auto loop = uvw::Loop::getDefault();

	// Create a resource that will listen to a pipe/unix socket
    auto pipe = loop->resource<uvw::PipeHandle>(false);



    pipe->on<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::PipeHandle &srv) {
        std::shared_ptr<uvw::PipeHandle> socket = srv.loop().resource<uvw::PipeHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PipeHandle &) { std::cout<<"FAIL"<<std::endl; });
        socket->on<uvw::CloseEvent>([&srv](const uvw::CloseEvent &, uvw::PipeHandle &) { srv.close(); std::cout<<"Server close"<<std::endl; });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) { sock.close(); std::cout<<"Socket close"<<std::endl; });
		socket->on<uvw::DataEvent>([](const uvw::DataEvent &evt, uvw::PipeHandle &sock){
				std::cout<<"Got something: "<<std::endl;
				std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
				sock.write(&evt.data[0], evt.length);
				});

        srv.accept(*socket);
		std::cout<<"Accepting connection from "<<socket->peer()<<", on "<<socket->sock()<<std::endl;
        socket->read();
    });

	pipe->bind(socket_path);
	pipe->listen();

	std::cout<<"Socket is \""<<pipe->sock()<<"\""<<std::endl;

    loop->run();
}
