module Jekyll
  class AppNameTag < Liquid::Tag
    def render(context)
      "<i>MakerVR</i>"
    end
  end
end

Liquid::Template.register_tag('appname', Jekyll::AppNameTag)
