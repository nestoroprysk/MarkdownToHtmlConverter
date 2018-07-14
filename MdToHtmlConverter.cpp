#include "MdToHtmlConverter.hpp"
#include <regex>

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
	std::string modifiedLine = lines_.getNextLine();
	modifiedLine = reaplaceHeader(modifiedLine);
	return modifiedLine;
}

bool MdToHtmlConverter::LinesStructurer::noMore() const
{
	return lines_.noMore();
}

std::string MdToHtmlConverter::LinesStructurer::reaplaceHeader(std::string line) const
{
	static const std::regex header("^#{1,6}[ ]*.$");
	if (std::regex_match(line, header)){
		const auto nbHashes = countHashes(line);
		line = std::regex_replace(line, header, "", std::regex_constants::format_first_only);
		const auto openTag = createOpenHeaderTag(line, nbHashes);
	}
	return line;
}

std::string MdToHtmlConverter::LinesStructurer::createOpenHeaderTag(std::string const& line, const size_t len) const
{
	std::string result("<h1 id=\"a\">");
	result.replace(0, 1, std::to_string(len));
	result.replace(8, 1, line);
	return result;
}


size_t MdToHtmlConverter::LinesStructurer::countHashes(std::string const& str) const
{
	for (size_t i = 0; i < str.size(); ++i)
		if (str[i] != '#')
			return i;
	return std::string::npos;
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
