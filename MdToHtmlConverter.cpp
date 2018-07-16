#include "MdToHtmlConverter.hpp"
#include <functional>

MdToHtmlConverter::MdToHtmlConverter(const char* fileName)
	: ifile_(fileName)
{
	if (!ifile_)
		throw std::ios_base::failure(
			"Something went wrong while trying to open the file.");
}

void MdToHtmlConverter::convert()
{
	auto lines = readLines(ifile_);
	auto paragraphs = makeParagraphs(lines);
	auto taggedParagraphs = tagParagraphs(paragraphs);
	auto taggedAndFormattedParagraphs =
		formatParagraphs(taggedParagraphs);
	writeResults(taggedAndFormattedParagraphs);
}

std::ofstream& operator<<(std::ofstream& o,
	MdToHtmlConverter::Paragraph const& p)
{
	for (auto const& line : p.lines)
		o << line << std::endl;
	return o;
}

MdToHtmlConverter::ParagraphMaker::ParagraphMaker(Reader& lines)
	: lines_(lines) {}

auto MdToHtmlConverter::ParagraphMaker::getParagraph() -> Paragraph
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

auto MdToHtmlConverter::ParagraphMaker::defineParagraphType(
	std::string const& line) -> ParagraphType
{
	for (size_t paragraphType = 0;
			paragraphType < nbCustomParagraphTypes_;
				++paragraphType)
		if (theSameParagraphType(line, static_cast<ParagraphType>(paragraphType)))
			return static_cast<ParagraphType>(paragraphType);
	return simpleText;
}

bool MdToHtmlConverter::ParagraphMaker::theSameParagraphType(
	std::string const& line, ParagraphType paragraphType)
{
	if (line.empty()) return false;
	static const std::regex headerPattern("^[ ]*#{1,6}[ ].+");
	static const std::regex unorderedListPattern("^[ ]*[*][ ].+");
	static const std::regex orderedListPattern("^[ ]*[0-9]+[.][ ].+");
	static std::regex paragraphTypePatterns[]
		{headerPattern, unorderedListPattern, orderedListPattern};
	if (paragraphType == simpleText){
		for (size_t allegedParagraphType = 0;
			allegedParagraphType < nbCustomParagraphTypes_;
				++allegedParagraphType)
			if (std::regex_match(line,
				paragraphTypePatterns[allegedParagraphType]))
				return false;
		return true;
	}
	return std::regex_match(line, paragraphTypePatterns[paragraphType]);
}

MdToHtmlConverter::ParagraphTagger::ParagraphTagger(
	ParagraphMaker& paragraphs)
		: paragraphs_(paragraphs) {}

auto MdToHtmlConverter::ParagraphTagger::getParagraph() -> Paragraph
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

auto MdToHtmlConverter::ParagraphTagger::tagHeader(
	Paragraph paragraph) -> Paragraph
{
	const auto nbHashes = countHashes(paragraph.lines[0]);
	paragraph.lines[0].erase(0, nbHashes + 1);
	paragraph.lines[0] =  createOpenHeaderTag(paragraph.lines[0], nbHashes) +
		paragraph.lines[0] + createCloseHeaderTag(nbHashes);
	return paragraph;
}

auto MdToHtmlConverter::ParagraphTagger::tagUnorderedList(
	Paragraph paragraph) -> Paragraph
{
	return tagList(paragraph, "*", "<ul>", "</ul>");
}

auto MdToHtmlConverter::ParagraphTagger::tagOrderedList(
	Paragraph paragraph) -> Paragraph
{
	return tagList(paragraph, ".", "<ol>", "</ol>");
}

auto MdToHtmlConverter::ParagraphTagger::tagList(
	Paragraph paragraph, std::string const& delimiter,
		std::string const& openTag, std::string const& closeTag) -> Paragraph
{
	size_t lastNbSpacesBeforeStar =  paragraph.lines[0].find(delimiter);
	size_t nbNestedLists = 0;
	for (size_t i = 0; i < paragraph.lines.size(); ++i){
		size_t currentNbSpacesBeforeStar = paragraph.lines[i].find(delimiter);
		if (currentNbSpacesBeforeStar > lastNbSpacesBeforeStar){
			++nbNestedLists;
			lastNbSpacesBeforeStar = currentNbSpacesBeforeStar;
			paragraph.lines.insert(paragraph.lines.begin() + i, openTag);
			++i;
		}
		else if (currentNbSpacesBeforeStar < lastNbSpacesBeforeStar){
			--nbNestedLists;
			lastNbSpacesBeforeStar = currentNbSpacesBeforeStar;
			paragraph.lines.insert(paragraph.lines.begin() + i, closeTag);
			++i;
		}
		paragraph.lines[i].erase(0, paragraph.lines[i].find(delimiter) + 2);
		paragraph.lines[i] = "<li>" + paragraph.lines[i] + "</li>";
	}
	for (size_t i = 0; i < nbNestedLists; ++i)
		paragraph.lines.insert(paragraph.lines.end(), closeTag);
	paragraph.lines.insert(paragraph.lines.begin(), openTag);
	paragraph.lines.push_back(closeTag);
	return paragraph;
}

MdToHtmlConverter::Paragraph MdToHtmlConverter::ParagraphTagger::tagSimpleText(
	Paragraph paragraph)
{
	paragraph.lines[0] =  "<p>" + paragraph.lines[0];
	paragraph.lines[paragraph.lines.size() - 1] += "</p>";
	return paragraph;
}

size_t MdToHtmlConverter::ParagraphTagger::countHashes(std::string const& str)
{
	size_t begin = 0;
	while (std::isspace(str[begin]))
		++begin;
	for (size_t i = begin; i < str.size(); ++i)
		if (str[i] != '#')
			return i - begin;
	return std::string::npos;
}

std::string MdToHtmlConverter::ParagraphTagger::createOpenHeaderTag(
	std::string const& line, const size_t len)
{
	return "<h" + std::to_string(len) + " id=\"" + line + "\">";
}

std::string MdToHtmlConverter::ParagraphTagger::createCloseHeaderTag(const size_t len)
{
	return "</h" + std::to_string(len) + ">";
}

MdToHtmlConverter::ParagraphFormatter::ParagraphFormatter(
	ParagraphTagger& paragraphs)
		: paragraphs_(paragraphs) {}

auto MdToHtmlConverter::ParagraphFormatter::getParagraph() -> Paragraph
{
	Paragraph paragraph = paragraphs_.getParagraph();
	static const std::regex boldSingPattern("[*]{2}");
	while (unformattedTextExists(paragraph, boldSingPattern))
		paragraph = makeFormatted(paragraph,
			"<strong>", "</strong>", "**");
	static const std::regex italicsPattern("[*]{1}");
	while (unformattedTextExists(paragraph, italicsPattern))
		paragraph = makeFormatted(paragraph,
			"<em>", "</em>", "*");
	static const std::regex strikeoutPattern("[~]{2}");
	while (unformattedTextExists(paragraph, strikeoutPattern))
		paragraph = makeFormatted(paragraph,
			"<del>", "</del>", "~~");
	return paragraph;
}

bool MdToHtmlConverter::ParagraphFormatter::noMore() const
{
	return paragraphs_.noMore();
}

bool MdToHtmlConverter::ParagraphFormatter::unformattedTextExists(
	Paragraph const& paragraph, std::regex const& pattern)
{
	size_t totalMatches = 0;
	for (auto const& line : paragraph.lines){
		const auto nbMatches(std::distance(
			std::sregex_iterator(line.begin(), line.end(), pattern),
			std::sregex_iterator()));
		totalMatches += nbMatches;
		if (totalMatches > 1)
			return true;
	}
	return false;
}

auto MdToHtmlConverter::ParagraphFormatter::makeFormatted(
	Paragraph paragraph, std::string const& openTag,
		std::string const& closeTag, std::string const& formatSign) -> Paragraph
{
	size_t nbModifications = 0;
	for (auto& line : paragraph.lines){
		auto boldSignIndex = line.find(formatSign);
		while (boldSignIndex != std::string::npos && nbModifications < 2){
			auto const& tag = (nbModifications % 2 == 0)
				? openTag : closeTag;
			line.replace(boldSignIndex, formatSign.size(), tag);
			++nbModifications;
			boldSignIndex = line.find(formatSign);
		}
		if (nbModifications > 1)
			break;
	}
	return paragraph;
}

Reader MdToHtmlConverter::readLines(std::ifstream& ifile)
{
	return Reader(ifile);
}

auto MdToHtmlConverter::makeParagraphs(Reader& lines) -> ParagraphMaker
{
	return ParagraphMaker(lines);
}

auto MdToHtmlConverter::tagParagraphs(
	ParagraphMaker& paragraphs) -> ParagraphTagger
{
	return ParagraphTagger(paragraphs);
}

auto MdToHtmlConverter::formatParagraphs(
	ParagraphTagger& paragraphs) -> ParagraphFormatter
{
	return ParagraphFormatter(paragraphs);
}

void MdToHtmlConverter::writeResults(ParagraphFormatter& paragraphs)
{
	std::ofstream ofile("result.html");
	while (!paragraphs.noMore())
		ofile << paragraphs.getParagraph();
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
