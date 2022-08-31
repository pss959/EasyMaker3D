module Jekyll
  class AppNameTag < Liquid::Tag
    def render(context)
      app_name = ENV.has_key?("APP_NAME") ? ENV["APP_NAME"] : "MakerVR"
      "<i>" + app_name + "</i>"
    end
  end
  class VersionTag < Liquid::Tag
    def render(context)
      ENV.has_key?("VERSION_STRING") ? ENV["VERSION_STRING"] : "X.Y.Z"
    end
  end
end

Liquid::Template.register_tag('appname', Jekyll::AppNameTag)
Liquid::Template.register_tag('version', Jekyll::VersionTag)
