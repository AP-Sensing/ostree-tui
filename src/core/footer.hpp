/*_____________________________________________________________
 | Footer Render
 |   Bottom portion of main window, includes a section for
 |   keyboard shortcuts info.
 |___________________________________________________________*/

#include "ftxui/component/component.hpp"        // for ftxui
#include "ftxui/component/component_base.hpp"   // for Component

class Footer {
public:
    const std::string DEFAULT_CONTENT   {"  || (Q)uit || (R)efresh || (C)opy commit hash || "};
    std::string content                 {DEFAULT_CONTENT};

public:
    Footer() = default;
    
    /// reset footer text to default string
    void resetContent();

    /// create a Renderer for the footer section
    ftxui::Element footerRender();
};
