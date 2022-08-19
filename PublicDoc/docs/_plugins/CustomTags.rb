module Jekyll
  class AppNameTag < Liquid::Tag
    def render(context)
      "<i>" + ENV["APP_NAME"] + "</i>"
    end
  end
  class VersionTag < Liquid::Tag
    def render(context)
      ENV["VERSION_STRING"]
    end
  end
end

Liquid::Template.register_tag('appname', Jekyll::AppNameTag)
Liquid::Template.register_tag('version', Jekyll::VersionTag)
