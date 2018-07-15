#include "MdToHtmlConverter.hpp"
#include <regex>
#include <functional>

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
	auto taggedParagraphs = tagParagraphs(paragraphs);
	writeResults(taggedParagraphs);
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
	static const std::regex headerPattern("^#{1,6}[ ].+");
	static const std::regex unorderedListPattern("^[*][ ].+");
	static const std::regex orderedListPattern("^[0-9]+[.][ ].+");
	static std::regex paragraphTypePatterns[] {headerPattern, unorderedListPattern, orderedListPattern};
	if (paragraphType == simpleText){
		for (size_t allegedParagraphType = 0; allegedParagraphType < nbCustomParagraphTypes_; ++allegedParagraphType)
			if (std::regex_match(line, paragraphTypePatterns[allegedParagraphType]))
				return false;
		return true;
	}
	return std::regex_match(line, paragraphTypePatterns[paragraphType]);
}

MdToHtmlConverter::ParagraphTagger::ParagraphTagger(ParagraphMaker& paragraphs)
	: paragraphs_(paragraphs) {}

MdToHtmlConverter::Paragraph MdToHtmlConverter::ParagraphTagger::getTaggedParagraph()
{
	Paragraph paragraph = paragraphs_.getParagraph();
	static const std::function<Paragraph(Paragraph)> tagParagraph[] =
		{ tagHeader, tagUnorderedList, tagOrderedList, tagSimpleText };
	return tagParagraph[static_cast<size_t>(paragraph.type)](paragraph);
}

bool MdToHtmlConverter::ParagraphTagger::noMore() const
{
	return paragraphs_.noMore();
}

MdToHtmlConverter::Paragraph MdToHtmlConverter::ParagraphTagger::tagHeader(Paragraph paragraph)
{
	const auto nbHashes = countHashes(paragraph.lines[0]);
	paragraph.lines[0].erase(0, nbHashes + 1);
	paragraph.lines[0] =  createOpenHeaderTag(paragraph.lines[0], nbHashes) + paragraph.lines[0] +
		createCloseHeaderTag(nbHashes);
	return paragraph;
}

MdToHtmlConverter::Paragraph MdToHtmlConverter::ParagraphTagger::tagUnorderedList(Paragraph paragraph)
{
	for (auto& word : paragraph.lines){
		word.erase(0, 2);
		word = "<li>" + word + "</li>";
	}
	paragraph.lines.insert(paragraph.lines.begin(), "<ul>");
	paragraph.lines.push_back("</ul>");
	return paragraph;
}

MdToHtmlConverter::Paragraph MdToHtmlConverter::ParagraphTagger::tagOrderedList(Paragraph paragraph)
{
	return paragraph;
}

MdToHtmlConverter::Paragraph MdToHtmlConverter::ParagraphTagger::tagSimpleText(Paragraph paragraph)
{
	paragraph.lines[0] =  "<p>" + paragraph.lines[0];
	paragraph.lines[paragraph.lines.size() - 1] += "</p>";
	return paragraph;
}

size_t MdToHtmlConverter::ParagraphTagger::countHashes(std::string const& str)
{
	for (size_t i = 0; i < str.size(); ++i)
		if (str[i] != '#')
			return i;
	return std::string::npos;
}

std::string MdToHtmlConverter::ParagraphTagger::createOpenHeaderTag(std::string const& line, const size_t len)
{
	return "<h" + std::to_string(len) + " id=\"" + line + "\">";
}

std::string MdToHtmlConverter::ParagraphTagger::createCloseHeaderTag(const size_t len)
{
	return "</h" + std::to_string(len) + ">";
}

Reader MdToHtmlConverter::readLines(std::ifstream& ifile)
{
	return Reader(ifile);
}

MdToHtmlConverter::ParagraphMaker MdToHtmlConverter::makeParagraphs(Reader& lines)
{
	return ParagraphMaker(lines);
}

MdToHtmlConverter::ParagraphTagger MdToHtmlConverter::tagParagraphs(ParagraphMaker& paragraphs)
{
	return ParagraphTagger(paragraphs);
}

void MdToHtmlConverter::writeResults(ParagraphTagger& paragraphs)
{
	std::ofstream ofile("result.html");
	while (!paragraphs.noMore())
		ofile << paragraphs.getTaggedParagraph() << std::endl;
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
