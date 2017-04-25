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

	//pipe->connect(socket_path);

	//{{{ pipe->connect(socket_path) with support for abstract sockets

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

	if( connect(sock.fd, static_cast<struct sockaddr*>(static_cast<void*>(&sock.addr)), sizeof(sock.addr.sun_family)+socket_path.length()) == -1 )
	{
		throw std::runtime_error("Could not connect to socket "+socket_path+".");
	}

	pipe->open(sock.fd);
	//}}}

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
