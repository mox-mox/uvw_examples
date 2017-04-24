#include <uvw.hpp>
#include <iostream>


// Listen to terminal input an echo it
int main()
{
	// Create a loop
    auto loop = uvw::Loop::getDefault();

	// Create a resource that will listen to STDIN
    auto console = loop->resource<uvw::TTYHandle>(uvw::StdIN, true);

	// Set the callback function
    console->on<uvw::DataEvent>([](auto& evt, auto& hndl){
			std::cout<<"Got something: "<<std::endl;
			// The event argument is a struct with only two members: A unique_ptr
			// to a char array and an integer with the length.
			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
    });
	console->read();

    loop->run();
}
