#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

# -----------------------------------------------------------------------------
# FixReplace event handler. Handles subsitutions inside directives.
# Adapted from:
#   https://github.com/sphinx-doc/sphinx/issues/4054
# -----------------------------------------------------------------------------

def FixReplace(app, docname, source):
  result = source[0]
  for key in app.config.fixreplace_dict:
    result = result.replace(key, app.config.fixreplace_dict[key])
    source[0] = result

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
   app.add_config_value('fixreplace_dict', {}, True)
   app.connect('source-read', FixReplace)
