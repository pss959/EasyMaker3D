// The version selector on each page allows the user to select a different
// version of the page. The other version is indicated with the string
// "_VERSION_X.Y.Z_". This code replaces all such strings with the specified
// version of the current page's URL. It is invoked when the page is loaded.

$("document").ready(function() {

    // Replaces the version in the given URL with the given version. A version
    // must be 3 numbers separated by dots and filling a component of the URL
    // path.
    function replaceVersionInURL(url, version) {
        const version_re = /^\d+\.\d+\.\d+$/;
        words = url.split('/');
        for (let i = 0; i < words.length; ++i) {
            if (version_re.test(words[i])) {
                return url.replace(words[i], version);
            }
        }
        throw new Error('*** Cannot find version in URL path');
    }

    // Search for the marker string in the document and replace each instance
    // with the correct version of the current URL.
    const marker_re = /_VERSION_(\d+\.\d+\.\d+)_/g;

    function replacer(match, new_version) {
        return replaceVersionInURL(document.URL, new_version)
    }

    document.body.innerHTML =
        document.body.innerHTML.replaceAll(marker_re, replacer);
});
