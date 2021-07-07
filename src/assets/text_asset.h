#pragma once

namespace Assets {

class Text
{
public:
	Text(const std::string& path);

	const std::string& GetContent() const;

private:
	std::string content;
};

}