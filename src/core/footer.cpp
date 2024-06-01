#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "footer.hpp"

ftxui::Element Footer::footerRender() {
	using namespace ftxui;
	
	return hbox({
		text(" OSTree TUI ") | bold,
		separator(),
		text(content) | (content == DEFAULT_CONTENT ? color(Color::White) : color(Color::YellowLight)),
	});
}

void Footer::resetContent() {
	content = DEFAULT_CONTENT;
}
