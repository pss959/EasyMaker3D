# -----------------------------------------------------------------------------
# The SConscript file sets these definitions in environment variables. It would
# be great if the "project" and "version" variables could be overridden from
# outside with the "-D" option, but that does not seem to work at all.
# -----------------------------------------------------------------------------

from os      import environ
from os.path import abspath
from sys     import path as sys_path

project = environ.get('PROJECT', '<Unknown Project>')
version = environ.get('VERSION', '<Unknown Version>')
release = version

# External links.
download_site = (
    'https://drive.google.com/drive/folders/1Z-ri3U1eGfifelhi-wnCQR6fk42etpde')

# -----------------------------------------------------------------------------
# Project information
# -----------------------------------------------------------------------------

copyright = '2022, Paul S. Strauss'
author    = 'Paul S. Strauss'

# -----------------------------------------------------------------------------
# General configuration.
# -----------------------------------------------------------------------------

# Add path to extensions.
sys_path.append(abspath("./_ext"))

extensions = [
  'incimage',
  'menuicon',
]

# Ignore included files.
exclude_patterns = [
    'init.rst-inc',
    'UserGuide/*.rst',
]

# Prolog includes a file with reStructuredText substitutions. Define some
# configuration subsitutions here first.
rst_prolog = f"""
.. |project|       replace:: {project}
.. |version|       replace:: {release}
.. _download_site: {download_site}
.. include:: /init.rst-inc
"""

# -----------------------------------------------------------------------------
# HTML output options.
# -----------------------------------------------------------------------------

html_theme       = 'sphinx_rtd_theme'  # Read the Docs.
html_static_path = ['_static']
html_css_files   = ['css/custom.css']
html_favicon     = 'images/favicon.ico'
# html_logo        = 'images/logo.jpg'
html_show_sourcelink = False
html_theme_options = {
    'display_version':             True,
    'logo_only':                   False,
    'prev_next_buttons_location':  'top',
    'style_external_links':        False,
    'style_nav_header_background': '#78a5e9',

    # -- Toc options
    'collapse_navigation':         True,
    'includehidden':               True,
    'navigation_depth':            4,
    'sticky_navigation':           True,
    'titles_only':                 False,
}
