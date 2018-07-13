#include "MdToHtmlConverter.hpp"
#include <iostream>

int main()
{
	try{
		MdToHtmlConverter converter("text.md");
		converter.convert();
		std::cout << "Check your result.html file!" << std::endl;
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
	}
}
