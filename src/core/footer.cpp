#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "footer.hpp"

ftxui::Component footer::footerRender() {
	using namespace ftxui;
	
	return ftxui::Renderer([] {
		return hbox({
			text(" OSTree TUI ") | bold,
			separator(),
			text("  || (Q)uit || (R)efresh || (C)opy commit hash ||  "),
		});
	});
}
