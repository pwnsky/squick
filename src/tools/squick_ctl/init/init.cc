
#include "init.h"


namespace squick_ctl::init {
	
	Init::Init() {
		std::cout << "Init\n";
	}

	Init::~Init() {

	}

	void Init::Exec() {
		std::cout << "Exec\n";
		auto files = Files::GetFileListInFolder("../", 1);

		for (auto& file : files) {
			std::cout << file << std::endl;
		}


	}

}
