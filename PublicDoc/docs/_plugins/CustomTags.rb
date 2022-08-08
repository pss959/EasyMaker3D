module Jekyll
  class AppNameTag < Liquid::Tag
    def render(context)
      "MakerVR"
    end
  end
end

Liquid::Template.register_tag('appname', Jekyll::AppNameTag)
