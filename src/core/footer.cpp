#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "footer.hpp"

ftxui::Element Footer::footerRender() {
	using namespace ftxui;
	
	return hbox({
		text("OSTree TUI") | bold | hyperlink("https://github.com/AP-Sensing/ostree-tui"),
		separator(),
		text(content) | (content == DEFAULT_CONTENT ? color(Color::White) : color(Color::YellowLight)),
	});
}

void Footer::resetContent() {
	content = DEFAULT_CONTENT;
}

void Footer::setContent(std::string content) {
	this->content = content;
}
