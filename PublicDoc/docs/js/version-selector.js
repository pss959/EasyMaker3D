window.addEventListener('DOMContentLoaded', function() {
    // Returns the currently-shown version as a string by parsing the given
    // path, looking for the segment after any of the given keys.
    function getVersionFromPath(path, keys) {
        words = path.split('/');
        var key_index = -100;
        for (let i = 0; i < words.length; ++i) {
            if (keys.includes(words[i]))
                key_index = i;
            else if (i == key_index + 1)
                return words[i];
        }
        throw new Error('*** Cannot find version in URL path');
    }

    function makeSelect(options, selected) {
        var select = document.createElement('select');
        select.classList.add('form-control');
        options.forEach(function(i) {
            var option = new Option(i.text, i.value, undefined,
                                    i.value === selected);
            select.add(option);
        });
        return select;
    }

    // Note: 'project_names' and 'versions' variables come from the versions.js
    // file. 'base_url' comes from the base.html template for the theme.
    var current_version = getVersionFromPath(window.location.pathname,
                                             project_names);

    var realVersion = versions.find(function(i) {
      return i.version === current_version ||
             i.aliases.includes(current_version);
    }).version;

    var select = makeSelect(versions.map(function(i) {
        return {text: 'Version ' + i.version +
                (i.aliases.length > 0 ? ' [' + i.aliases + ']' : ''),
                value: i.version};
    }), realVersion);
    select.addEventListener('change', function(event) {
        window.location.href = base_url + '/../' + this.value;
    });

    var container = document.createElement('div');
    container.id = 'version-selector';
    container.appendChild(select);

    var title = document.querySelector('.navbar-brand');
    if (title.parentNode.classList.contains('navbar-header')) {
      var height = window.getComputedStyle(title).getPropertyValue('height');
      container.style.height = height;
    }

    title.parentNode.insertBefore(container, title.nextSibling);
});
