// The version selector on each page allows the user to select a different
// version of the page. This populates it with all versions found in the
// "versions.js" file by replacing the marker '__VERSION_SELECTOR__' string with
// the version choices.
//
// This function is invoked when any page is loaded.

$("document").ready(function() {

    // Replaces the version string in the given URL with a '__VERSION__' marker
    // string. The version string must be 3 numbers separated by dots and
    // filling a component of the URL path.
    function storeMarkerInURL(url) {
        const version_re = /^\d+\.\d+\.\d+$/;
        words = url.split('/');
        for (let i = 0; i < words.length; ++i) {
            if (version_re.test(words[i])) {
                return url.replace(words[i], '__VERSION__');
            }
        }
        throw new Error('*** Cannot find version in URL path');
    }

    // Replace the version in the current page's URL with the marker string.
    marked_url = storeMarkerInURL(document.URL);

    // Builds a selection string for the given version.
    function buildSelectionString(version) {
        sel = '<dd><a href="' + marked_url.replace('__VERSION__',
                                                   version.version) +
            '">Version ' + version.version;
        if (version.alias) {
            sel += ' [' + version.alias + ']';
        }
        sel += '</a>';
        return sel;
    }

    // Create a string with version selection for each version.
    selections = ''
    for (const version of all_versions) {
        selections += buildSelectionString(version);
    }

    // Replace the marker string in the document with the version selection
    // string.
    selector = document.getElementById('rst-version-selector');
    selector.innerHTML = selector.innerHTML.replace('__VERSION_SELECTOR__',
                                                    selections);
});
