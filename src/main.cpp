#include <iostream>

#include <globals.h>
#include <config.h>
#include <otf_manager.h>

int main(const int argc, const char* argv[])
{
	int error {0};

	try
	{
		std::shared_ptr<Config> cfg = std::make_shared<Config>(argc, argv);

		OTF_Manager manager(cfg);
		manager.read_otf();
	}
	catch (const std::invalid_argument &e)
	{
		std::cout << "invalid_argument: " << e.what() << std::endl;
		error = 1;
	}
	catch (const std::bad_alloc &e)
	{
		std::cout << "bad_alloc: " << e.what() << std::endl;
		error = 2;
	}
	catch (const std::runtime_error &e)
	{
		std::cout << "runtime_error: " << e.what() << std::endl;
		error = 3;
	}
	catch (const std::exception &e)
	{
		std::cout << "exception: " << e.what() << std::endl;
		error = 4;
	}
	catch (...)
	{
		std::cout << "Unknown error!" << "\n";
		error = 255;
	}

	return error;
}
