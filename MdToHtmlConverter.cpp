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
	auto paragraphs = makeParagraphs(lines);
	writeResults(paragraphs);
}

std::ofstream& operator<<(std::ofstream& o, MdToHtmlConverter::Paragraph const& p)
{
	o << '{' << static_cast<int>(p.type) << '}';
	for (auto const& line : p.lines)
		o << line << '@';
	return o;
}

MdToHtmlConverter::ParagraphMaker::ParagraphMaker(Reader& lines)
	: lines_(lines) {}

MdToHtmlConverter::Paragraph MdToHtmlConverter::ParagraphMaker::getParagraph()
{
	Paragraph paragraph;
	{
		std::string firstLine = (lineFromLastReading_.empty())
		? lines_.getNextLine() : lineFromLastReading_;
		paragraph.type = defineParagraphType(firstLine);
		paragraph.lines.push_back(firstLine);
	}
	std::string line = lines_.getNextLine();
	while (theSameParagraphType(line, paragraph.type)){
		paragraph.lines.push_back(line);
		line = lines_.getNextLine();
	}
	lineFromLastReading_ = line;
	return paragraph;
}

bool MdToHtmlConverter::ParagraphMaker::noMore() const
{
	return lines_.noMore();
}

MdToHtmlConverter::ParagraphType MdToHtmlConverter::ParagraphMaker::defineParagraphType(std::string const& line)
{
	for (size_t paragraphType = 0; paragraphType < nbCustomParagraphTypes_; ++paragraphType)
		if (theSameParagraphType(line, static_cast<ParagraphType>(paragraphType)))
			return static_cast<ParagraphType>(paragraphType);
	return simpleText;
}

bool MdToHtmlConverter::ParagraphMaker::theSameParagraphType(std::string const& line, ParagraphType paragraphType)
{
	if (line.empty()) return false;
	static const std::regex headerPattern("^[ ]*#{1,6}[ ].+");
	static const std::regex unorderedListPattern("^[ ]*[*][ ].+");
	static const std::regex orderedListPattern("^[ ]*[0-9]+[.][ ].+");
	static std::regex paragraphTypePatterns[] {headerPattern, unorderedListPattern, orderedListPattern};
	if (paragraphType == simpleText){
		for (size_t allegedParagraphType = 0; allegedParagraphType < nbCustomParagraphTypes_; ++allegedParagraphType)
			if (std::regex_match(line, paragraphTypePatterns[allegedParagraphType]))
				return false;
		return true;
	}
	return std::regex_match(line, paragraphTypePatterns[paragraphType]);
}

Reader MdToHtmlConverter::readLines(std::ifstream& ifile)
{
	return Reader(ifile);
}

MdToHtmlConverter::ParagraphMaker MdToHtmlConverter::makeParagraphs(Reader& lines)
{
	return ParagraphMaker(lines);
}

void MdToHtmlConverter::writeResults(ParagraphMaker& paragraphs)
{
	std::ofstream ofile("result.html");
	while (!paragraphs.noMore())
		ofile << paragraphs.getParagraph() << std::endl;
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

// bool beginningOfNextPara(std::string const& line)
// {
// 	if (line.empty()) return true;
// 	static const std::regex header("^[ ]*#{1,6}[ ]");
// 	static const std::regex header("^[ ]*#{1,6}[ ]");
// }

// std::string MdToHtmlConverter::ParagraphMaker::replaceHeader(std::string line)  // obsolete
// {
// 	const auto nbHashes = countHashes(line);
// 	line.erase(0, nbHashes + 1);
// 	line = createOpenHeaderTag(line, nbHashes) + line + createCloseHeaderTag(nbHashes);
// 	return line;
// }

// std::string MdToHtmlConverter::ParagraphMaker::markSimpleParagraph(std::string line)
// {
// 	return "<p>" + line + "</p>";
// }

// size_t MdToHtmlConverter::ParagraphMaker::countHashes(std::string const& str)
// {
// 	for (size_t i = 0; i < str.size(); ++i)
// 		if (str[i] != '#')
// 			return i;
// 	return std::string::npos;
// }

// std::string MdToHtmlConverter::ParagraphMaker::createOpenHeaderTag(std::string const& line, const size_t len)
// {
// 	return "<h" + std::to_string(len) + " id=\"" + line + "\">";
// }

// std::string MdToHtmlConverter::ParagraphMaker::createCloseHeaderTag(const size_t len)
// {
// 	return "</h" + std::to_string(len) + ">";
// }
