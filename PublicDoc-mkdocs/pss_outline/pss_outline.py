'''
Outline extension for Python-Markdown
=====================================

Derived from outdated mdx_outline extension; fixed to work with Python 3.9
and mkdocs >= 1.2.1
'''

from markdown                import Extension
from markdown.treeprocessors import Treeprocessor
from markdown.util           import etree

__version__ = "1.0.0"

# For debugging...
def _EltToStr(elt):
    text = ''
    if elt.text:
        n = min(20, len(elt.text))
        text = elt.text[0:n].strip().replace('\n', ' ')
    return f'<{elt.tag}> {hex(id(elt))} "{text}"'

def _DumpTree(when, root):
    def _DumpElt(elt, level):
        print('  ' * level, _EltToStr(elt))
        for child in elt:
            _DumpElt(child, level + 1)
    print(f'TREE {when}--------------------------------------------')
    _DumpElt(root, 0)
    print(f'-------------------------------------------------------')

class OutlineProcessor(Treeprocessor):
    def run(self, root):
        #_DumpTree('BEFORE', root)
        new_root = self._ProcessTree(root)
        #_DumpTree('AFTER', new_root)
        return new_root

    def _ProcessTree(self, root):
        # Stack of elements at each level as (level, section_elt). The first
        # entry is to avoid having to check for an empty stack.
        self._level_stack = [(0, None)]

        # Cannot do this destructively, since iterating over the children of
        # root would fail horribly. Create a new tree.
        new_root = etree.Element(root.tag, attrib=root.attrib)

        for child in root:
            level = self._GetHeaderLevel(child.tag)
            if level <= 0:
                # Not a header? Add to the current section, if any.
                self._MoveToTopSection(new_root, child)
            else:  # It's a header.
                # If at the same or shallower level, end the current section(s).
                while level < self._CurLevel():
                    self._EndSection()
                # Start a new section.
                self._StartSection(new_root, level, child)

        # Exit remaining sections.
        while self._CurLevel() > 0:
            self._EndSection()

        return new_root

    def _GetHeaderLevel(self, tag):
        if (len(tag) == 2 and tag[0].lower() == 'h'
            and tag[1] > '0' and tag[1] <= '6'):
            return ord(tag[1]) - ord('0')
        else:
            return 0

    def _StartSection(self, parent, level, child):
        section = etree.Element('section',
                                attrib={'class': f'section{level}'})
        self._AddTo(parent, section)
        self._AddTo(section, child)
        self._level_stack.append((level, section))
        return section

    def _EndSection(self):
        self._level_stack.pop()

    def _MoveToTopSection(self, root, elt):
        top_section = self._level_stack[-1][1]
        self._AddTo(top_section if top_section else root, elt)

    def _CurLevel(self):
        return self._level_stack[-1][0]

    def _AddTo(self, parent, child):
        parent.append(child)

class OutlineExtension(Extension):
    def __init__(self, *args, **kwargs):
        super(OutlineExtension, self).__init__(**kwargs)

    def extendMarkdown(self, md):
        ext = OutlineProcessor(md)
        md.treeprocessors.register(ext, 'outline', 0)

def makeExtension(*args, **kwargs):
    return OutlineExtension(kwargs)
