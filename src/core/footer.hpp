/*_____________________________________________________________
 | Footer Render
 |   Bottom portion of main window, includes a section for
 |   keyboard shortcuts info.
 |___________________________________________________________*/

class Footer {
   public:
    Footer() = default;

    /// reset footer text to default string
    void resetContent();

    /// create a Renderer for the footer section
    ftxui::Element footerRender();

    // Setter
    void setContent(std::string content);

   private:
    const std::string DEFAULT_CONTENT{
        "  || Alt+Q : Quit || Alt+R : Refresh || Alt+C : Copy commit hash || Alt+P : Promote "
        "Commit "};
    std::string content{DEFAULT_CONTENT};
};
