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
