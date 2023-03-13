# -----------------------------------------------------------------------------
# The SConscript file sets these definitions in environment variables. It would
# be great if the "project" and "version" variables could be overridden from
# outside with the "-D" option, but that does not seem to work at all.
# -----------------------------------------------------------------------------

from os      import environ
from os.path import abspath
from sys     import path as sys_path

project = environ.get('PROJECT',   '<Unknown Project>')
version = environ.get('VERSION',   '<Unknown Version>')
suffix  = environ.get('SUFFIX',    '<Unknown Suffix>')  # For session files.
copyrt  = environ.get('COPYRIGHT', '<Unknown Copyright>')
baseurl = environ.get('BASE_URL',  '<Unknown Base URL>')
release = version

# External links.
download_site = (
    'https://drive.google.com/drive/folders/1Z-ri3U1eGfifelhi-wnCQR6fk42etpde')

# -----------------------------------------------------------------------------
# Project information
# -----------------------------------------------------------------------------

copyright = copyrt
author    = 'Paul S. Strauss'

# -----------------------------------------------------------------------------
# Version selection.
# -----------------------------------------------------------------------------

# Runs git with the given arguments, returning the resulting lines that match
# the given pattern as a list.
def RunGit_(arg_string, pattern):
    from subprocess import Popen, PIPE, run
    lines = Popen(['git'] + arg_string.split(),
                  stdout=PIPE, text=True).stdout.read()
    return [s for line in lines.split() if pattern in (s := line.strip())]

# Determine all versions to include. Use the last tag for each release branch.
# Create a dictionary mapping version to URL.
rels = [rel.replace('Release-', '') for rel in RunGit_('branch', 'Release-')]
tags = RunGit_('tag',    'v')

latest_tags = []
for rel in rels:
    latest_tags.append([t for t in tags if t.startswith('v' + rel + '.')][-1])

doc_versions = [tag.replace('v', '') for tag in latest_tags]
version_dict = dict((vers, f'{baseurl}/{vers}/') for vers in doc_versions)

# -----------------------------------------------------------------------------
# General configuration.
# -----------------------------------------------------------------------------

# Add path to extensions.
sys_path.append(abspath("./_ext"))

extensions = [
    # Official extensions:
    'sphinx.ext.todo',

    # Local extensions:
    'fixreplace',
    'incimage',
    'menuicon',
]

# Ignore included files.
exclude_patterns = [
    'init.rst',
]

# Prolog includes a file with reStructuredText substitutions. Define some
# configuration subsitutions here first and include the init file that sets up
# roles and substitutions.
rst_prolog = f"""
.. |project| replace:: {project}
.. |version| replace:: {release}
.. |suffix|  replace:: {suffix}
.. _download_site: {download_site}
.. include:: /init.rst
"""

templates_path = ['_templates']

todo_include_todos = True

# Substitutions that can be used inside code blocks and other directives:
fixreplace_dict = {
    "{appname}" : f'{project}',
}

# -----------------------------------------------------------------------------
# HTML output options.
# -----------------------------------------------------------------------------

html_theme           = 'sphinx_rtd_theme'  # Read the Docs.
html_static_path     = ['_static']
html_css_files       = ['css/custom.css']
html_js_files        = ['js/custom.js']
html_favicon         = 'images/static/favicon.ico'
# html_logo          = 'images/logo.jpg'
html_show_sourcelink = False
html_theme_options   = {
    'display_version':             False,  # Use version selector instead.
    'logo_only':                   False,
    'prev_next_buttons_location':  'both',
    'style_external_links':        False,
    'style_nav_header_background': '#78a5e9',

    # -- Toc options
    'collapse_navigation':         True,
    'includehidden':               True,
    'navigation_depth':            -1,
    'sticky_navigation':           True,
    'titles_only':                 False,
}

# Variables passed to templates.
html_context = {
    'current_version' : version,
    'version_dict'    : version_dict,
}
