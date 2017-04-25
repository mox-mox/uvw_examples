#include <uvw.hpp>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>

std::string socket_path("\0echo.sock", sizeof("\0echo.sock")-1);
//std::string socket_path("echo.sock", sizeof("echo.sock")-1);

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

	//pipe->bind(socket_path);

	//{{{ pipe->bind(socket_path) with support for abstract sockets

	struct Sock
	{
		int fd;
		struct sockaddr_un addr;
	}sock;
	if((sock.fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}
	//std::cout<<"....................FD: "<<sock.fd<<std::endl;

	sock.addr.sun_family = AF_UNIX;


	if(socket_path.length() >= sizeof(sock.addr.sun_path)-1)
	{
		throw std::length_error("Unix socket path \"" + socket_path + "\" is too long. "
		                         "Maximum allowed size is " + std::to_string(sizeof(sock.addr.sun_path)) + "." );
	}

	socket_path.copy(sock.addr.sun_path, socket_path.length());

  	// Unix sockets beginning with a null character map to the invisible unix socket space.
  	// Since Strings that begin with a null character a difficult to handle, use % instead
  	// and translate % to the null character here.
	if(sock.addr.sun_path[0] == '%') sock.addr.sun_path[0] = '\0';

	if(bind(sock.fd, static_cast<struct sockaddr*>(static_cast<void*>(&sock.addr)), sizeof(sock.addr.sun_family)+socket_path.length()) == -1 )
	{
		return -1;
	}

	pipe->open(sock.fd);
	//}}}


	pipe->listen();

	std::cout<<"Socket is \""<<pipe->sock()<<"\""<<std::endl;

    loop->run();
}
