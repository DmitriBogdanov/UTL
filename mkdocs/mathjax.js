// __________________________________ CONTENTS ___________________________________
// 
//    This is a standard JS include for the MathJax LaTeX support provided by the
//    MkDocs Material: https://squidfunk.github.io/mkdocs-material/reference/math/
// _______________________________________________________________________________

window.MathJax = {
    tex: {
        inlineMath: [["\\(", "\\)"]],
        displayMath: [["\\[", "\\]"]],
        processEscapes: true,
        processEnvironments: true
    },
    options: {
        ignoreHtmlClass: ".*|",
        processHtmlClass: "arithmatex"
    }
};

document$.subscribe(() => {
    MathJax.startup.output.clearCache()
    MathJax.typesetClear()
    MathJax.texReset()
    MathJax.typesetPromise()
});
