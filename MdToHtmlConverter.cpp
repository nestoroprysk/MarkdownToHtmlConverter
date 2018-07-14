#include "MdToHtmlConverter.hpp"

MdToHtmlConverter::MdToHtmlConverter(const char* fileName)
	: ifile_(fileName)
{
	if (!ifile_)
		throw std::ios_base::failure("Something went wrong while trying to open the file.");
}

void MdToHtmlConverter::convert()
{
	auto lines = readLines(ifile_);
	auto structuredLines = structureLines(lines);
	writeResults(structuredLines);
}

MdToHtmlConverter::LinesStructurer::LinesStructurer(Reader& lines)
	: lines_(lines) {}

std::string MdToHtmlConverter::LinesStructurer::getParagraph()
{
	return lines_.getNextLine() + " <header>";
}

bool MdToHtmlConverter::LinesStructurer::noMore() const
{
	return lines_.noMore();
}

Reader MdToHtmlConverter::readLines(std::ifstream& ifile)
{
	return Reader(ifile);
}

MdToHtmlConverter::LinesStructurer MdToHtmlConverter::structureLines(Reader& lines)
{
	return LinesStructurer(lines);
}

void MdToHtmlConverter::writeResults(LinesStructurer& structuredLines)
{
	std::ofstream ofile("result.html");
	while (!structuredLines.noMore())
		ofile << structuredLines.getParagraph() << std::endl;
}

Reader::Reader(std::ifstream& ifile)
	: ifile_(ifile) {}

std::string Reader::getNextLine()
{
	std::string line;
	std::getline(ifile_, line);
	if (!ifile_)
		eof_ = true;
	return line;
}

bool Reader::noMore() const
{
	return eof_;
}
