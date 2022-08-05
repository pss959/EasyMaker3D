import itertools

# Defines macros used in documentation.
def define_env(env):
  "Hook function"

  # Inline image.
  @env.macro
  def inlineimage(height):
      return f'{{: style="height:{height}px;" }}'

  # Right-justified image with the given height.
  @env.macro
  def rightimage(height):
      return f'{{: style="float:right; margin:2px; height:{height}px;" }}'

  # Special case for images of tool icons.
  @env.macro
  def toolimage():
      return f'{{: style="float:left; margin-right:8px; height:80px;" }}'

  # Vertical space (usually for an image).
  @env.macro
  def verticalspace(amount):
      return '<br>'.join(itertools.repeat('&nbsp;', amount)) + '\n'

  # Introducing a new term
  @env.macro
  def term(text):
      return f'<span class="term">{text}</span>'

  # Tiny text.
  @env.macro
  def tiny(text):
      return f'<span class="tiny">{text}</span>'

  # Colored text.
  @env.macro
  def color(color_string, text):
      return f'<span style="color:{color_string};">{text}</span>'

  # Named anchor. Put this before the text to anchor.
  @env.macro
  def anchor(name):
      return f'<a id="{name}"></a>'

  # Multiple named anchors (list). Put this before the text to anchor.
  @env.macro
  def anchors(names):
      return ''.join(anchor(name) for name in names)
