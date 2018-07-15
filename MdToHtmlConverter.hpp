#pragma once

#include <fstream>
#include <vector>

class Reader;

class MdToHtmlConverter
{
public:
	MdToHtmlConverter(const char* fileName);
	void convert();
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
		Paragraph getTaggedParagraph();
		bool noMore() const;
	private:
		static Paragraph tagHeader(Paragraph);
		static Paragraph tagUnorderedList(Paragraph);
		static Paragraph tagOrderedList(Paragraph);
		static Paragraph tagSimpleText(Paragraph);
		static size_t countHashes(std::string const&);
		static std::string createOpenHeaderTag(std::string const& line, const size_t len);
		static std::string createCloseHeaderTag(const size_t len);
		ParagraphMaker& paragraphs_;
	};
	Reader readLines(std::ifstream&);
	ParagraphMaker makeParagraphs(Reader&);
	ParagraphTagger tagParagraphs(ParagraphMaker&);
	void writeResults(ParagraphTagger&);
	std::ifstream ifile_;
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
