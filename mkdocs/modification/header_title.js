// __________________________________ CONTENTS ___________________________________
// 
//    This is a workaround script for setting the header title text separately
//    from the site title. Site title is used for a whole bunch of things including
//    the navigation page and tab title, so this is the easiest point of separation.
//    We use this add a longer description since there is plenty of space available.
// _______________________________________________________________________________

(function (){
    if (window.innerWidth <= 1224) return;
    // don't append long description on narrow viewpoints, this doesn't update
    // on window resize so it's mainly here to filter out mobile devices
    
    let headers = [...document.getElementsByClassName('md-header__ellipsis')]
    if (headers.length === 0) return;
    let header  = headers[0] 
    
    let titles = header.children;
    if (titles.length === 0) return;
    let title  = titles[0]
    
    let spans = title.children;
    if (spans.length === 0) return;
    let span = spans[0];
    
    if (span.innerHTML.includes('UTL')) {
        let separator = document.createElement('span');
        separator.setAttribute('class', 'md-ellipsis');
        separator.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
        // HTML collapses repeating spaces so we use non-breaking-space chars instead, aligning
        // with spaces is a very questionable way, but it works well enough for the purpose
        
        title.appendChild(separator);
                
        let description = document.createElement('span');
        description.setAttribute('class', 'md-ellipsis');
        description.innerHTML = 'Collection of self-contained header-only libraries for C++17';
        
        title.appendChild(description);
    }
})();
