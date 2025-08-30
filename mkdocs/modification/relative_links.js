// __________________________________ CONTENTS ___________________________________
// 
//    Modification script which adjusts '<- back to README.md' links so they point
//    to home page instead of README when viewed though a website, without this we
//    would just get 404 (since README resides outside of the docs folder).
// _______________________________________________________________________________

// Redirect README links to the homepage
//
[...document.querySelectorAll("a[href='..'")]
    .forEach(el => {
        const homepage_link = 'index.html'
        el.setAttribute('href', homepage_link)
        
        console.log('Replaced link', '..', 'with', homepage_link)
    });

// Adjust relative links that go beyond the doc files to link Github repo instead
//
// This way the the docs can be entirely functional as a local / repo files,  
// yet automatically work the way we would expect when deployed as a website
//
// The algorithm here is somewhat complicated in order to work with files in
// subdirectories (which may include several '../' without going "out" of the website)
//
function count_occurrences(str, substr) {
    return str.split(substr).length - 1
}
    
{
    const repo_link   = 'https://github.com/DmitriBogdanov/UTL/blob/master/';
    
    const path_prefix   = '/UTL/';
    const path_relative = document.location.pathname.toString().substring(path_prefix.length);
    const path_level    = count_occurrences(path_relative, '/');
     
    [...document.querySelectorAll("a[href^='..'")] // ^= => "begins with" selector
        .forEach(el => {
            const href              = el.getAttribute('href')
            const href_return_level = count_occurrences(href, '..')
           
            if (href_return_level > path_level) {
                const ignore_size   = '../'.length * href_return_level
                const href_relative = el.getAttribute('href').substring(ignore_size) 
                el.setAttribute('href', repo_link + href_relative)
            }
        });
}
// [...document.querySelectorAll("a[href^='..'")] // ^= => "begins with" selector
//     .forEach(el => {
//         const repo_link     = 'https://github.com/DmitriBogdanov/UTL/blob/master'
//         const relative_link = el.getAttribute('href').substring(2)
//         el.setAttribute('href', repo_link + relative_link)
        
//         console.log('Replaced link', '..' + relative_link, 'with', repo_link + relative_link)
//     });