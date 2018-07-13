#pragma once

#include <fstream>

class Reader;

class MdToHtmlConverter
{
public:
	MdToHtmlConverter(const char* fileName);
	void convert();
private:
	class HeaderMarker
	{
	public:
		HeaderMarker(Reader&);
		std::string getNextLine();
		bool noMore() const;
	private:
		Reader& lines_;
	};
	// ...
	class PragraphMarker
	{
	public:
		PragraphMarker(HeaderMarker&);
		std::string getNextLine();
		explicit operator bool() const;
	private:
		HeaderMarker& linesWithMarkedHeaders_;
	};
	Reader readLines(std::ifstream&);
	HeaderMarker markHeaders(Reader&);
	PragraphMarker markParagraphs(HeaderMarker&);
	void writeResults(PragraphMarker&);
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
