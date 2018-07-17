#pragma once

#include <fstream>
#include <vector>
#include <regex>

class Reader;

class MdToHtmlConverter
{
public:
	static void convert(const char* fileName);
	MdToHtmlConverter() = delete;
private:
	enum ParagraphType { header, unorderedList, orderedList, simpleText };
	struct Paragraph
	{
		ParagraphType type = simpleText;
		std::vector<std::string> lines;
	};
	friend std::ofstream& operator<<(std::ofstream&, Paragraph const&);
	class ParagraphMaker
	{
	public:
		ParagraphMaker(Reader&);
		Paragraph getParagraph();
		bool noMore() const;
	private:
		static ParagraphType defineParagraphType(std::string const&);
		static bool theSameParagraphType(std::string const&, ParagraphType);
		static const size_t nbCustomParagraphTypes_ = 3;
		Reader& lines_;
		std::string lineFromLastReading_;
	};
	class ParagraphTagger
	{
	public:
		ParagraphTagger(ParagraphMaker&);
		Paragraph getParagraph();
		bool noMore() const;
	private:
		static Paragraph tagHeader(Paragraph);
		static Paragraph tagUnorderedList(Paragraph);
		static Paragraph tagOrderedList(Paragraph);
		static Paragraph tagList(Paragraph, std::string const& delimiter,
			std::string const& openTag, std::string const& closeTag);
		static Paragraph tagSimpleText(Paragraph);
		static size_t countHashes(std::string const&);
		static std::string createOpenHeaderTag(std::string const& line,
			const size_t len);
		static std::string createCloseHeaderTag(const size_t len);
		ParagraphMaker& paragraphs_;
	};
	class ParagraphFormatter
	{
	public:
		ParagraphFormatter(ParagraphTagger&);
		Paragraph getParagraph();
		bool noMore() const;
	private:
		static bool unformattedTextExists(Paragraph const&, std::regex const&);
		static Paragraph makeFormatted(Paragraph, std::string const& openTag,
			std::string const& closeTag, std::string const& formatSign);
		ParagraphTagger& paragraphs_;
	};
	static Reader readLines(std::ifstream&);
	static ParagraphMaker makeParagraphs(Reader&);
	static ParagraphTagger tagParagraphs(ParagraphMaker&);
	static ParagraphFormatter formatParagraphs(ParagraphTagger&);
	static void writeResults(ParagraphFormatter&);
};

class Reader
{
public:
	Reader(std::ifstream&);
	std::string getNextLine();
	bool noMore() const;
private:
	std::ifstream& ifile_;
	bool eof_ = false;
};
