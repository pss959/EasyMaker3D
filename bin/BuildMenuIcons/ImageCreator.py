# -----------------------------------------------------------------------------
# ImageCreator class that wraps ImageDraw calls.
# -----------------------------------------------------------------------------

from PIL  import Image, ImageChops, ImageDraw, ImageFilter, ImageFont, ImageOps
from math import cos, radians, sin

from BuildMenuIcons.Point import Point
from BuildMenuIcons.Rect  import Rect

class ImageCreator(object):
    # -------------------------------------------------------------------------
    # Default state values.
    # -------------------------------------------------------------------------

    DefaultLineWidth = 6

    # -------------------------------------------------------------------------
    # Initialization and saving.
    # -------------------------------------------------------------------------

    def __init__(self, size, background):
        self._image = Image.new("RGBA", (size, size), background)
        self._draw  = ImageDraw.Draw(self._image)
        self._line_width = ImageCreator.DefaultLineWidth
        self._font_size  = 0
        self._dash_ratio = 2  # Ratio of dash length to space length.
        self.SetFontSize(15)

    def Save(self, filebase):
        self._image.save(filebase + '.png', "PNG")

    # -------------------------------------------------------------------------
    # Setting state.
    # -------------------------------------------------------------------------

    def SetLineWidth(self, width):
        self._line_width = width

    def SetFontSize(self, points):
        if self._font_size != points:
            self._font_size = points
            self._font = ImageFont.truetype("arialbd.ttf", self._font_size)

    def SetDashRatio(self, ratio):
        self._dash_ratio = ratio

    # -------------------------------------------------------------------------
    # Drawing.
    # -------------------------------------------------------------------------

    def Arc(self, rect, start_degrees, end_degrees, color, fill=False):
        xy = [rect.min.Tuple(), rect.max.Tuple()]
        if fill:
            self._draw.chord(xy, start_degrees, end_degrees, fill=color)
        else:
            self._draw.arc(xy, start_degrees, end_degrees,
                           fill=color, width=self._line_width)

    def Circle(self, center, radius, color, fill=False):
        self.Ellipse(center, radius, radius, color, fill)

    def DashedLine(self, p0, p1, color):
        # Can handle only horizontal and vertical lines.
        assert(p0.x == p1.x or p0.y == p1.y)
        if p0.x == p1.x:  # Vertical
            self._DashedVLine(p0.x, p0.y, p1.y, color)
        else:
            self._DashedHLine(p0.x, p1.x, p0.y, color)

    def DashedPolyLine(self, pts, color):
        for i in range(len(pts) - 1):
            self.DashedLine(pts[i], pts[i + 1], color)

    def Ellipse(self, center, xradius, yradius, color, fill=False):
        self.EllipseFromRect(Rect.FromMinMax(center - Point(xradius, yradius),
                                             center + Point(xradius, yradius)),
                             color, fill)

    def EllipseFromRect(self, rect, color, fill=False):
        if fill:
            self._draw.ellipse([rect.min.Tuple(), rect.max.Tuple()],
                               fill=color, outline=None)
        else:
            self._draw.ellipse([rect.min.Tuple(), rect.max.Tuple()],
                               outline=color, width=self._line_width)

    def Line(self, p0, p1, color):
        self._draw.line([p0.Tuple(), p1.Tuple()], fill=color,
                        width=self._line_width, joint='curve')

    def PolyLine(self, pts, color):
        self._draw.line([p.Tuple() for p in pts], fill=color,
                        width=self._line_width, joint='curve')

    def Polygon(self, pts, color, fill=False):
        if fill:
             self._draw.polygon([p.Tuple() for p in pts], fill=color)
        else:
            self.PolyLine(pts, color)

    def Rectangle(self, rect, color, fill=False):
        if fill:
            self._draw.rectangle(rect.ToList(), fill=color, outline=None)
        else:
            self._draw.rectangle(rect.ToList(), outline=color,
                                 width=self._line_width)

    def RegularPolygon(self, center, radius, sides, color,
                       start_angle=0, fill=False):
        assert(sides >= 3)
        points = self._RegularPolygonPoints(center, radius, sides, start_angle)
        self.Polygon(points, color, fill)

    def RoundedRectangle(self, rect, radius, color, fill=False):
        assert(2 * radius < rect.size.x and 2 * radius < rect.size.y)
        points = self._RoundedRectanglePoints(rect, radius)
        self.Polygon(points, color, fill)

    def Text(self, center, text, color):
        # Set anchor to 'mm': middle/middle.
        self._draw.text(center.Tuple(), text, fill=color, font=self._font,
                        anchor='mm')

    # -------------------------------------------------------------------------
    # Image operations.
    # -------------------------------------------------------------------------

    def AddImage(self, other_ic):
        self._image = ImageChops.add(self._image, other_ic._image)

    def Blur(self, radius=2):
        self._image = self._image.filter(ImageFilter.GaussianBlur(radius))

    def Mirror(self):
        self._image = ImageOps.mirror(self._image)

    # -------------------------------------------------------------------------
    # Implementation.
    # -------------------------------------------------------------------------

    def _DashedHLine(self, x0, x1, y, color):
        dash_size = self._GetDashSize()
        skip_size = self._GetSkipSize()
        xl  = min(x0, x1)
        xr = max(x0, x1)
        x = xl
        while xl < xr:
            x = min(xr, xl + dash_size)
            self._draw.line([(xl, y), (x, y)], fill=color,
                            width=self._line_width)
            xl = x + skip_size

    def _DashedVLine(self, x, y0, y1, color):
        dash_size = self._GetDashSize()
        skip_size = self._GetSkipSize()
        yt = min(y0, y1)
        yb = max(y0, y1)
        y = yt
        while yt < yb:
            y = min(yb, yt + dash_size)
            self._draw.line([(x, yt), (x, y)], fill=color,
                            width=self._line_width)
            yt = y + skip_size

    def _GetDashSize(self):
        return self._dash_ratio * self._line_width

    def _GetSkipSize(self):
        return self._line_width

    def _RegularPolygonPoints(self, center, radius, sides, start_angle):
        delta_angle = radians(float(360) / sides)
        points = []
        angle = radians(start_angle)
        for i in range(sides):
            points.append(center + self._CirclePoint(radius, angle))
            angle += delta_angle
        points.append(points[0])
        return points

    def _RoundedRectanglePoints(self, rect, radius):
        points = []
        # Move around clockwise (since that is how arcs are iterated when Y is
        # increasing downward), starting at the top left, post-radius.
        points.append(rect.TL() + Point(radius, 0))
        # Top right, pre-radius.
        points.append(rect.TR() - Point(radius, 0))
        # Top right arc.
        center = rect.TR() + Point(-radius, radius)
        points += [center + p for p in self._ArcPoints(radius, 270, 90)]
        # Bottom right, pre-radius.
        points.append(rect.BR() - Point(0, radius))
        # Bottom right arc.
        center = rect.BR() + Point(-radius, -radius)
        points += [center + p for p in self._ArcPoints(radius, 0, 90)]
        # Bottom left, pre-radius.
        points.append(rect.BL() + Point(radius, 0))
        # Bottom left arc.
        center = rect.BL() + Point(radius, -radius)
        points += [center + p for p in self._ArcPoints(radius, 90, 90)]
        # Top left, pre-radius.
        points.append(rect.TL() + Point(0, radius))
        # Top left arc.
        center = rect.TL() + Point(radius, radius)
        points += [center + p for p in self._ArcPoints(radius, 180, 90)]
        return points

    def _ArcPoints(self, radius, start_angle, arc_angle, delta_angle=10):
        angle = radians(start_angle)
        end_angle = radians(start_angle + arc_angle)
        delta = radians(delta_angle)
        points = []
        while angle <= end_angle:
            points.append(self._CirclePoint(radius, angle))
            angle += delta
        return points

    def _CirclePoint(self, radius, angle_in_radians):
        return Point(int(radius * cos(angle_in_radians)),
                     int(radius * sin(angle_in_radians)))
