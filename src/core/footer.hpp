/*_____________________________________________________________
 | Footer Render
 |   Bottom portion of main window, includes a section for
 |   keyboard shortcuts info.
 |___________________________________________________________*/

class Footer {
   public:
    Footer() = default;

    /// @brief Resets footer text to default string.
    void ResetContent();

    /// @brief Creates a Renderer for the footer section.
    ftxui::Element FooterRender();

    // Setter
    void SetContent(std::string content);

   private:
    const std::string DEFAULT_CONTENT{
        "  || Alt+Q : Quit || Alt+R : Refresh || Alt+C : Copy Hash || Alt+P : Promote || Alt+D: "
        "Drop || "};
    std::string content{DEFAULT_CONTENT};
};
