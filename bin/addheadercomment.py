#!/usr/bin/env python3

#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

# -----------------------------------------------------------------------------
# Adds the correct copyright/licensing header comment to each source file
# (including this one!) if it is not already there. The set of files to operate
# on is baked into this script.
#
# This should be run in the root project directory.
# -----------------------------------------------------------------------------

from os      import walk
from os.path import join
from fnmatch import filter

# -----------------------------------------------------------------------------
# Configuration variables.
# -----------------------------------------------------------------------------

# This is the header that is added as a comment:
header_comment = ('SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss\n'
                  'SPDX-License-Identifier: AGPL-3.0-or-later')

# This marker is used to make it easy to detect previous versions of the header
# so they can be replaced.
marker = '@@@@'

# Directories to search for source files.
dirs = ['bin', 'resources', 'src', 'PublicDoc']

# For each type of file suffix, this defines: the suffix and comment string for
# files with that suffix.
file_info = [
  ['cpp', '//'],
  ['emd', '#'],
  ['ems', '#'],
  ['h',   '//'],
  ['off', '#'],
  ['py',  '#'],
]

# -----------------------------------------------------------------------------
# Builds and returns the comment lines for the given comment_chars. Lines all
# end in a newline.
# -----------------------------------------------------------------------------

def GetCommentLines(comment_chars):
  # Prepend the comment string and surround with marker lines and add a blank
  # line.
  marker_line = comment_chars + marker + '\n'
  comment_lines = []
  comment_lines.append(marker_line)
  comment_lines += [comment_chars + ' ' + line + '\n'
                    for line in header_comment.split('\n')]
  comment_lines.append(marker_line)
  comment_lines.append('\n')
  return comment_lines

# -----------------------------------------------------------------------------
# Returns paths to files under the given directory that match the given suffix.
# -----------------------------------------------------------------------------

def GetFilePaths(directory, suffix):
  paths = []
  for root, dirnames, filenames in walk(directory):
    paths += [join(root, fn) for fn in filter(filenames, f'*.{suffix}')]
  return sorted(paths);

# -----------------------------------------------------------------------------
# Returns the number of lines to skip when inserting the header comment for the
# given file path and contents.
# -----------------------------------------------------------------------------

def GetLinesToSkip(file_path, file_lines):
  # If this is a Python file with a hashbang, skip it and the next (blank)
  # line.
  if (file_path.endswith('.py') and
      file_lines and file_lines[0].startswith('#!')):
    return 2
  else:
    return 0

# -----------------------------------------------------------------------------
# Checks if the lines from a file already contain the given comment lines in
# the right place. If so, this returns (True, 0). If it contains a different
# version of the comment lines, this returns (False, n), where n is the number
# of previous comment lines (including the marker lines). Otherwise, it returns
# (False, 0).
# -----------------------------------------------------------------------------

def HasCommentLines(file_lines, comment_lines, lines_to_skip):
  # Not enough lines to contain the comment.
  if len(file_lines) < lines_to_skip + len(comment_lines):
    return (False, 0)

  # If there is a marker line in the correct place, collect all lines up to the
  # line after the next marker line (inclusive) into a list. Otherwise, leave
  # the list empty.
  file_comment_lines = []
  if file_lines[lines_to_skip] == comment_lines[0]:
    file_comment_lines.append(comment_lines[0])
    i = lines_to_skip + 1
    while i < len(file_lines) and file_lines[i] != comment_lines[0]:
      file_comment_lines.append(file_lines[i])
      i = i + 1
    if i < len(file_lines):
      file_comment_lines.append(file_lines[i])  # Closing marker line.
      i = i + 1
    if i < len(file_lines):
      file_comment_lines.append(file_lines[i])  # Blank line.
      i = i + 1

  if file_comment_lines == comment_lines:
    return (True, 0)
  elif not file_comment_lines:
    return (False, 0)
  else:
    return (False, len(file_comment_lines))

# -----------------------------------------------------------------------------
# If the given file already has the given comment in the right place, this does
# nothing. Otherwise, it inserts the comment after removing any previous
# version if necessary.
# -----------------------------------------------------------------------------

def AddOrReplaceComment(file_path, comment_lines):
  with open(file_path,'r') as file:
    file_lines = file.readlines()

  lines_to_skip = GetLinesToSkip(file_path, file_lines)

  (has_comment, count) = HasCommentLines(file_lines, comment_lines,
                                           lines_to_skip)
  if not has_comment:
    if count > 0:
      print(f'== Replacing comment in {file_path}')
    else:
      print(f'== Adding comment to {file_path}')
    file_lines[lines_to_skip:lines_to_skip + count] = comment_lines

    with open(file_path, 'w') as file:
      file.writelines(file_lines)

# -----------------------------------------------------------------------------
# Mainline
# -----------------------------------------------------------------------------

for fi in file_info:
  suffix        = fi[0]
  comment_chars = fi[1]

  # Prepend the comment string and surround with marker lines and add a blank
  # line.
  comment_lines = GetCommentLines(comment_chars)

  for directory in dirs:
    for file_path in GetFilePaths(directory, suffix):
      AddOrReplaceComment(file_path, comment_lines)
