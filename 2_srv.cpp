#include <uvw.hpp>
#include <iostream>

std::string socket_path("\0echo.sock", sizeof("\0echo.sock")-1);

int main()
{
	// Create a loop
    auto loop = uvw::Loop::getDefault();

	// Create a resource that will listen to a pipe/unix socket
    auto pipe = loop->resource<uvw::PipeHandle>(false);


    pipe->on<uvw::ListenEvent>([](uvw::ListenEvent& evt, uvw::PipeHandle& hndl){
			(void) hndl;
			(void) evt;
			std::cout<<"Got something: "<<std::endl;
			// The event argument is a struct with only two members: A unique_ptr
			// to a char array and an integer with the length.
			//std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
    });

    pipe->on<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::PipeHandle &srv) {
        std::shared_ptr<uvw::PipeHandle> socket = srv.loop().resource<uvw::PipeHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PipeHandle &) { std::cout<<"FAIL"<<std::endl; });
        socket->on<uvw::CloseEvent>([&srv](const uvw::CloseEvent &, uvw::PipeHandle &) { srv.close(); std::cout<<"Server close"<<std::endl; });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) { sock.close(); std::cout<<"Socket close"<<std::endl; });
		socket->on<uvw::DataEvent>([](auto& evt, auto&){
				std::cout<<"Got something: "<<std::endl;
				// The event argument is a struct with only two members: A unique_ptr
				// to a char array and an integer with the length.
				std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
				});

        srv.accept(*socket);
        socket->read();

        //uvw::Addr local = srv.sock();
        //uvw::Addr remote = socket->peer();

        //ASSERT_EQ(local.ip, "127.0.0.1");
        //ASSERT_EQ(remote.ip, "127.0.0.1");
        //ASSERT_EQ(local.port, decltype(local.port){4242});
        //ASSERT_NE(remote.port, decltype(remote.port){0});
    });
















	pipe->bind(socket_path);
	pipe->listen();

	std::cout<<"Socket is \""<<pipe->sock()<<"\""<<std::endl;

    loop->run();
}
