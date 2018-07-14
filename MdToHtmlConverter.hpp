#pragma once

#include <fstream>

class Reader;

class MdToHtmlConverter
{
public:
	MdToHtmlConverter(const char* fileName);
	void convert();
private:
	class LinesStructurer
	{
	public:
		LinesStructurer(Reader&);
		std::string getParagraph();
		bool noMore() const;
	private:
		static std::string replaceHeader(std::string);
		static std::string markSimpleParagraph(std::string);
		static size_t countHashes(std::string const&);
		static std::string createOpenHeaderTag(std::string const& line, const size_t len);
		static std::string createCloseHeaderTag(const size_t len);
		Reader& lines_;
	};
	Reader readLines(std::ifstream&);
	LinesStructurer structureLines(Reader&);
	void writeResults(LinesStructurer&);
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
