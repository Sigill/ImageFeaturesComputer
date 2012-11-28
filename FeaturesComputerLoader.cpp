#include "FeaturesComputerLoader.h"

#include <dlfcn.h>
#include <sstream>

FeaturesComputerLoader::FeaturesComputerLoader(const std::string name)
{
	std::ostringstream plugin_file;
	plugin_file << "lib" << name << ".so";

	void* plug = dlopen(plugin_file.str().c_str(), RTLD_LAZY);
	if (!plug) {
		std::stringstream err;
		err << "Cannot load library " << name << ": " << dlerror();

		throw FeaturesComputerLoadingException(err.str());
	}

	// reset errors
	dlerror();

	this->module = plug;

	// load the create symbols
	create_t* create_plug = (create_t*) dlsym(this->module, "create");
	const char* dlsym_error = dlerror();
	if (dlsym_error) {
		std::stringstream err;
		err << "Cannot load symbol create: " << dlerror();
		dlclose(this->module);

		throw FeaturesComputerLoadingException(err.str());
	}

	// create an instance of the class
	this->computer = create_plug();
}

FeaturesComputerLoader::~FeaturesComputerLoader()
{
	if(this->computer) {
		delete this->computer;
		this->computer = NULL;
	}
	if(this->module) {
		dlclose(this->module);
		this->module = NULL;
	}
}
