#include "MdToHtmlConverter.hpp"
#include <iostream>

int main()
{
	try{
		MdToHtmlConverter::convert("sample.md");
		std::cout << "Check your result.html file!" << std::endl;
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
	}
}
