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
	auto linesWithMarkedHeaders = markHeaders(lines);
	auto linesWithMarkedHeadersAndParagraps =
		markParagraphs(linesWithMarkedHeaders);
	writeResults(linesWithMarkedHeadersAndParagraps);
}

MdToHtmlConverter::HeaderMarker::HeaderMarker(Reader& reader)
	: lines_(reader) {}

std::string MdToHtmlConverter::HeaderMarker::getNextLine()
{
	return lines_.getNextLine() + " <header>";
}

bool MdToHtmlConverter::HeaderMarker::noMore() const
{
	return lines_.noMore();
}

MdToHtmlConverter::PragraphMarker::PragraphMarker(HeaderMarker& headerMarker)
	: linesWithMarkedHeaders_(headerMarker) {}

std::string MdToHtmlConverter::PragraphMarker::getNextLine()
{
	return linesWithMarkedHeaders_.getNextLine() + " <para>";
}

MdToHtmlConverter::PragraphMarker::operator bool() const
{
	return !linesWithMarkedHeaders_.noMore();
}

Reader MdToHtmlConverter::readLines(std::ifstream& ifile)
{
	return Reader(ifile);
}

MdToHtmlConverter::HeaderMarker MdToHtmlConverter::markHeaders(Reader& lines)
{
	return HeaderMarker(lines);
}

MdToHtmlConverter::PragraphMarker MdToHtmlConverter::markParagraphs(HeaderMarker& linesWithMarkedHeaders)
{
	return PragraphMarker(linesWithMarkedHeaders);
}

void MdToHtmlConverter::writeResults(PragraphMarker& linesWithMarkedHeadersAndParagraps)
{
	std::ofstream ofile("result.html");
	while (linesWithMarkedHeadersAndParagraps)
		ofile << linesWithMarkedHeadersAndParagraps.getNextLine() << std::endl;
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
