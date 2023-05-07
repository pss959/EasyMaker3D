from math import cos, radians, sin

from BuildMenuIcons.ImageCreator import ImageCreator
from BuildMenuIcons.Point        import Point
from BuildMenuIcons.Rect         import Rect

# -----------------------------------------------------------------------------
# Basic settings.
# -----------------------------------------------------------------------------

# Set this to True when testing icons - it uses a light background to make the
# icons more visible.
TESTING = False

# -----------------------------------------------------------------------------
# IconBuilder class that builds specific menu icons.
# -----------------------------------------------------------------------------

class IconBuilder(object):
    # -------------------------------------------------------------------------
    # Public interface.
    # -------------------------------------------------------------------------

    def __init__(self):
        self._SetConstants()

    def List(self):
        print(' '.join(self._ListIcons()))

    def BuildAll(self):
        for icon in self._ListIcons():
              self.Build(icon)

    def Build(self, icon_name):
        ic = ImageCreator(self._FullSize, self._Background)
        ic.SetLineWidth(self._LineWidth)
        func = getattr(self, '_' + icon_name)
        func(ic)
        ic.Save(icon_name)

    # -------------------------------------------------------------------------
    # Returns a list of all icon names.
    # -------------------------------------------------------------------------

    def _ListIcons(self):
        method_list = [func for func in dir(self)
                       if callable(getattr(self, func)) and
                       func.startswith('_MI')]
        # Remove '_' at beginning of each method name.
        return [method[1:] for method in sorted(method_list)]

    # -------------------------------------------------------------------------
    # Sets up constants.
    # -------------------------------------------------------------------------

    def _SetConstants(self):
        # Width and height of each icon image and Rect for it.
        self._FullSize = 128
        self._HalfSize = self._FullSize / 2
        self._FullRect = Rect.FromSize(Point(self._FullSize, self._FullSize))

        # "Safe" size within icon image and Rect for it.
        self._SafeSize = self._FullSize - 20
        self._SafeRect = self._FullRect.Center(Point(self._SafeSize,
                                                     self._SafeSize))

        # Default background color.
        self._Background = (0, 0, 0, 0)
        if TESTING:
            self._Background = '#eeeeee'   # Use this only for testing.

        # Colors.
        self._CSGColor1        = "#77aaee"
        self._CSGColor2        = "#77aa66"
        self._CSGResColor      = "#ee88aa"
        self._CreateColor      = "#4444ee"
        self._GeneralColor     = "#223366"
        self._HighlightColor   = "#ee5555"
        self._LightColor       = "#ccd4ea"
        self._MenuColor        = "#6655ee"
        self._LayoutColor      = "#55aabb"
        self._SelectArrowColor = "#bb3333"
        self._SelectColor      = "#995566"
        self._ToolColor        = "#6a4595"

        # Default line width.
        self._LineWidth = 6

        # Rectangles for various icon-specific features.
        self._CreationPlusRect = Rect.FromCenterAndSize(
            Point(25, self._HalfSize), Point(36, 36))
        self._CreationObjRect = Rect.FromCenterAndSize(
            Point(84, self._HalfSize), Point(64, 64))
        self._CSGRect1 = Rect.FromMinMax(Point(10, 50), Point(90, 120))
        self._CSGRect2 = Rect.FromMinMax(Point(40, 10), Point(118, 100))
        self._SelectRect = self._SafeRect.Shrink(10)
        self._ToggleRect = Rect.FromCenterAndSize(Point(self._HalfSize, 102),
                                                  Point(self._HalfSize, 24))

    # -------------------------------------------------------------------------
    # Specific Icon building functions.
    # -------------------------------------------------------------------------

    def _MIColorTool(self, ic):
        c = self._SafeRect.center
        rc = c + Point(  0, -20)
        gc = c + Point(-20,  20)
        bc = c + Point( 20,  20)
        rad = 30
        # Composite translucent colors to form overlapping circles.
        ic.Circle(rc, rad, '#ff222288', fill=True)
        ic2 = ImageCreator(self._FullSize, (0, 0, 0, 0))
        ic2.Circle(gc, rad, '#22ff2288', fill=True)
        ic.AddImage(ic2)
        ic2 = ImageCreator(self._FullSize, (0, 0, 0, 0))
        ic2.Circle(bc, rad, '#2222ff88', fill=True)
        ic.AddImage(ic2)

    def _MICombineCSGDifference(self, ic):
        lw = self._LineWidth
        r1 = self._CSGRect1
        r2 = self._CSGRect2
        # Result is in 2 pieces; shrink them inside lines.
        rr1 = Rect.FromMinMax(r1.TL(), Point(r2.min.x, r1.max.y))
        rr2 = Rect.FromMinMax(r2.BL(), r1.BR())
        rr1 = rr1.MoveLeftEdge(lw).MoveTopEdge(lw).MoveBottomEdge(-lw)
        rr2 = rr2.MoveBottomEdge(-lw).MoveRightEdge(-lw)
        self._AddCSG(ic, [rr1, rr2])

    def _MICombineCSGIntersection(self, ic):
        rr = self._CSGRect1.Overlap(self._CSGRect2)
        self._AddCSG(ic, [rr.Shrink(self._LineWidth)])

    def _MICombineCSGUnion(self, ic):
        rr1 = self._CSGRect1.Shrink(self._LineWidth)
        rr2 = self._CSGRect2.Shrink(self._LineWidth)
        self._AddCSG(ic, [rr1, rr2])

    def _MICombineHull(self, ic):
        inner = self._SafeRect.Shrink(20)
        sq0 = inner.Bottom(30).Left(30)
        sq1 = inner.Bottom(30).Right(30)
        sq2 = inner.Top(30).Center(Point(30, 30))
        off = 6
        hull = [sq0.BL() + Point(-off,  off),
                sq1.BR() + Point( off,  off),
                sq1.TR() + Point( off,    0),
                sq2.TR() + Point(   0, -off),
                sq2.TL() + Point(   0, -off),
                sq0.TL() + Point(-off,    0)
        ]
        hull.append(hull[0])  # Close it.
        ic.SetLineWidth(6)
        ic.Rectangle(sq0, self._GeneralColor)
        ic.Rectangle(sq1, self._GeneralColor)
        ic.Rectangle(sq2, self._GeneralColor)
        ic.PolyLine(hull, self._HighlightColor)

    def _MIComplexityTool(self, ic):
        ic.SetLineWidth(4)
        lr = self._SafeRect.Left(40)
        rr = self._SafeRect.Right(40)
        ic.RegularPolygon(lr.center, lr.size.x / 2, 5, self._ToolColor)
        ic.RegularPolygon(rr.center, rr.size.x / 2, 8, self._ToolColor)
        # Arrow
        ic.SetLineWidth(2)
        lp = lr.CR() + Point(6, 0)
        rp = rr.CL() - Point(6, 0)
        arrow_end0 = Point(4,  4)
        arrow_end1 = Point(4, -4)
        ic.Line(lp, rp, self._ToolColor)
        ic.PolyLine([lp + arrow_end0, lp, lp + arrow_end1], self._ToolColor)
        ic.PolyLine([rp - arrow_end0, rp, rp - arrow_end1], self._ToolColor)

    def _MIConvertBevel(self, ic):
        ic.SetLineWidth(6)
        r = self._SafeRect.Shrink(10)
        ic.PolyLine([r.TL(), r.TR(), r.BR()], self._GeneralColor)
        ic.PolyLine([r.BR(), r.TR() + Point(-30, 30), r.TL()],
                    self._HighlightColor)

    def _MIConvertClip(self, ic):
        # Model rectangle.
        mr = self._SafeRect.Shrink(20)
        # Endpoints of clipping line.
        lc = self._SafeRect.CL() + Point(0,  10)
        rc = self._SafeRect.CR() + Point(0, -10)
        # Points where clipping line intersects model rectangle.
        li = lc + Point( 20, -5)
        ri = rc + Point(-20,  0)
        # Bottom of rectangle is drawn with solid lines.
        ic.PolyLine([li, mr.BL(), mr.BR(), ri], self._ToolColor)
        # Top of rectangle is drawn with dashed lines.
        ic.SetDashRatio(1)
        ic.DashedPolyLine([li, mr.TL(), mr.TR(), ri], self._ToolColor)
        # Clipping line.
        ic.SetLineWidth(8)
        ic.Line(lc, rc, self._HighlightColor)

    def _MIConvertMirror(self, ic):
        r = self._SafeRect.MoveTopEdge(20)
        ic.RegularPolygon(r.center, 32, 8, self._ToolColor)
        ic.DashedLine(r.TC(), r.BC(), self._HighlightColor)
        ar = self._SafeRect.Top(50).Center(Point(80, 50))
        ic.Arc(ar, 180, 360, self._HighlightColor)
        ap0 = ar.CL()
        ap1 = ar.CR()
        ic.PolyLine([ap0 + Point(-4, -14), ap0, ap0 + Point(14, 0)],
                    self._HighlightColor)
        ic.PolyLine([ap1 + Point(4, -14), ap1, ap1 + Point(-14, 0)],
                    self._HighlightColor)

    def _MIConvertTaper(self, ic):
        r = self._SafeRect.Shrink(10)
        bot_offset = Point(3, 0)
        top_offset = Point(10, 0)
        lt = r.TC() - top_offset
        rt = r.TC() + top_offset
        # Solid lines.
        ic.Line(r.BL(), r.BR(), self._ToolColor)
        ic.Line(lt,     rt,     self._ToolColor)
        # Dashed lines.
        ic.DashedPolyLine([r.BL(), r.TL(), lt], self._ToolColor)
        ic.DashedPolyLine([r.BR(), r.TR(), rt], self._ToolColor)
        # Taper points.
        ic.Line(r.BL() + bot_offset, r.TC() - top_offset, self._HighlightColor)
        ic.Line(r.BR() - bot_offset, r.TC() + top_offset, self._HighlightColor)

    def _MIConvertTwist(self, ic):
        cr = self._SafeRect.Center(Point(40, 80))
        self._AddCylinder(ic, cr, self._GeneralColor)
        ar = Rect(self._SafeRect.center - Point(0, 10), Point(80, 50))
        ic.Arc(ar, 20, 160, self._HighlightColor)
        ap0 = ar.CL() + Point(2, 3)
        ap1 = ar.CR() + Point(-2, 3)
        ic.PolyLine([ap0 + Point(-4, 14), ap0, ap0 + Point(14, 0)],
                    self._HighlightColor)
        ic.PolyLine([ap1 + Point(4, 14), ap1, ap1 + Point(-14, 0)],
                    self._HighlightColor)

    def _MICopy(self, ic):
        frontr = self._SafeRect.Center(Point(60, 80)).Move(Point(-10, 10))
        backr  = self._SafeRect.Center(Point(60, 80)).Move(Point(10, -5))
        ic.RoundedRectangle(backr,  6, self._GeneralColor)
        ic.RoundedRectangle(frontr, 6, self._Background, fill=True)
        ic.RoundedRectangle(frontr, 6, self._GeneralColor)

    def _MICreateBox(self, ic):
        edge = int(self._SafeSize / 2)
        cor = self._CreationObjRect
        self._AddPlus(ic)
        # Rect for front and back faces.
        frontr = cor.Left(edge).Bottom(edge)
        backr  = cor.Right(edge).Top(edge)
        # Front face. Enlarge a little so that the lines match up.
        lw2 = self._LineWidth / 2
        ic.Rectangle(frontr.MoveLeftEdge(-lw2).MoveBottomEdge(lw2),
                     self._CreateColor)
        # Connectors and part of back face.
        ic.PolyLine([frontr.TL(), backr.TL(), backr.TR(),
                     backr.BR(), frontr.BR()], self._CreateColor)
        # Middle Connector.
        ic.Line(frontr.TR(), backr.TR(), self._CreateColor)

    def _MICreateCylinder(self, ic):
        self._AddPlus(ic)
        self._AddCylinder(ic, self._CreationObjRect, self._CreateColor)

    def _MICreateExtruded(self, ic):
        edge = int(self._SafeSize / 2)
        cor = self._CreationObjRect
        self._AddPlus(ic)
        # Lines for base of model.
        bc = cor.BC() - Point(10, 0)
        ic.PolyLine([cor.CL(), bc, cor.CR()], self._CreateColor)
        # Lines for top of model.
        tc = cor.center - Point(10, 0)
        ic.PolyLine([cor.TL(), tc, cor.TR(), cor.TL()], self._HighlightColor)
        # Lines joining along extrusion direction.
        ic.SetLineWidth(3)
        ic.DashedLine(cor.TL(), cor.CL(), self._HighlightColor)
        ic.DashedLine(cor.TR(), cor.CR(), self._HighlightColor)
        ic.DashedLine(tc,       bc,       self._HighlightColor)

    def _MICreateImportedModel(self, ic):
        self._AddFileIcon(ic)
        # Add arrow.
        ap = self._SafeRect.center + Point(-10, 10)
        ic.Line(ap, ap + Point(60, 0), self._HighlightColor)
        ic.PolyLine([ap + Point(10, -10), ap, ap + Point(10, 10)],
                    self._HighlightColor)

    def _MICreateRevSurf(self, ic):
        self._AddPlus(ic)
        ic.SetLineWidth(5)
        r = self._CreationObjRect
        revPts = [(0,  4), (30, 5), (34, 20), (20, 30),
                  (35, 40), (36
                             , 50), (14, 58), (0,  60),]
        ic.PolyLine([r.TC() + Point(p[0], p[1]) for p in revPts],
                    self._GeneralColor)
        ic.PolyLine([r.TC() + Point(-p[0], p[1]) for p in revPts],
                    self._LightColor)
        ic.SetLineWidth(3)
        ic.DashedLine(r.TC(), r.BC(), self._HighlightColor)

    def _MICreateSphere(self, ic):
        self._AddPlus(ic)
        ic.Circle(self._CreationObjRect.center, self._CreationObjRect.size.x / 2,
                  self._CreateColor)

    def _MICreateText(self, ic):
        ic.SetFontSize(self._SafeSize - 10)
        ic.Text(self._SafeRect.center, 'T', self._GeneralColor)

    def _MICreateTorus(self, ic):
        self._AddPlus(ic)
        self._AddTorus(ic, self._CreationObjRect, self._CreateColor)

    def _MICut(self, ic):
        hole_center0 = self._SafeRect.BL() + Point(20, -50)
        hole_center1 = self._SafeRect.BL() + Point(50, -20)
        hole_radius = 14
        end0 = self._SafeRect.TR() + Point(-10, 40)
        end1 = self._SafeRect.TR() + Point(-40, 10)
        ic.SetLineWidth(10)
        ic.Line(hole_center0, end0, self._GeneralColor)
        ic.Line(hole_center1, end1, self._GeneralColor)
        ic.SetLineWidth(6)
        ic.Circle(hole_center0, hole_radius, self._Background, fill=True)
        ic.Circle(hole_center1, hole_radius, self._Background, fill=True)
        ic.Circle(hole_center0, hole_radius, self._GeneralColor)
        ic.Circle(hole_center1, hole_radius, self._GeneralColor)

    def _MIDecreaseComplexity(self, ic):
        c = self._SafeRect.center
        ic.RegularPolygon(c, self._SafeRect.size.x / 2, 9, self._GeneralColor)
        # Minus sign
        ic.Line(c - Point(20, 0), c + Point(20, 0), self._GeneralColor)

    def _MIDecreasePrecision(self, ic):
        # Minus sign
        msize = 14
        c = self._SafeRect.CL() + Point(msize, 10)
        ic.Line(c - Point(msize, 0), c + Point(msize, 0), self._GeneralColor)
        # Calipers
        cr = self._SafeRect.Shrink(10).Right(60)
        self._AddCalipers(ic, cr, self._GeneralColor)

    def _MIDelete(self, ic):
        ic.SetFontSize(self._SafeSize - 10)
        ic.Text(self._SafeRect.center, 'X', self._GeneralColor)

    def _MIHideSelected(self, ic):
        self._AddSelectRect(ic, self._GeneralColor)
        self._AddEye(ic, size=72)
        # Slash through eye
        ic.SetLineWidth(8)
        r = self._SafeRect.Shrink(24)
        ic.Line(r.TR(), r.BL(), self._HighlightColor)
        ic.Blur(1)

    def _MIIncreaseComplexity(self, ic):
        c = self._SafeRect.center
        ic.RegularPolygon(c, self._SafeRect.size.x / 2, 9, self._GeneralColor)
        # Plus sign
        ic.Line(c - Point(20, 0), c + Point(20, 0), self._GeneralColor)
        ic.Line(c - Point(0, 20), c + Point(0, 20), self._GeneralColor)

    def _MIIncreasePrecision(self, ic):
        # Plus sign
        psize = 14
        c = self._SafeRect.CL() + Point(psize, 10)
        ic.Line(c - Point(psize, 0), c + Point(psize, 0), self._GeneralColor)
        ic.Line(c - Point(0, psize), c + Point(0, psize), self._GeneralColor)
        # Calipers
        cr = self._SafeRect.Shrink(10).Right(60)
        self._AddCalipers(ic, cr, self._GeneralColor)

    def _MILinearLayout(self, ic):
        r = self._SafeRect.Shrink(12)
        # 3 cylinders
        rs = Point(20, 24)
        r0 = Rect.FromCenterAndSize(r.CL(),   rs)
        r1 = Rect.FromCenterAndSize(r.center, rs)
        r2 = Rect.FromCenterAndSize(r.CR(),   rs)
        ic.SetLineWidth(3)
        self._AddCylinder(ic, r0, self._LayoutColor)
        self._AddCylinder(ic, r1, self._LayoutColor)
        self._AddCylinder(ic, r2, self._LayoutColor)
        # Lines connecting cylinders
        ic.SetLineWidth(5)
        ic.Line(r0.CR(), r1.CL(), self._LayoutColor)
        ic.Line(r1.CR(), r2.CL(), self._LayoutColor)

    def _MIMoveNext(self, ic):
        self._AddMoveName(ic, 'up')

    def _MIMovePrevious(self, ic):
        self._AddMoveName(ic, 'down')

    def _MIMoveToOrigin(self, ic):
        # Axes
        ic.SetLineWidth(3)
        (xmin, ymin) = self._SafeRect.BL().Tuple()
        (xmax, ymax) = self._SafeRect.TR().Tuple()
        orig = self._SafeRect.BL() + Point(10, -10)
        ic.Line(Point(xmin, orig.y), Point(xmax, orig.y), self._GeneralColor)
        ic.Line(Point(orig.x, ymin), Point(orig.x, ymax), self._GeneralColor)
        # Dot at origin.
        ic.Circle(orig, 3, self._GeneralColor, fill=True)
        # Model
        mr = self._SafeRect.Shrink(10).Top(40).Right(40)
        self._AddCylinder(ic, mr, self._GeneralColor)
        # Arrow.
        ic.SetLineWidth(4)
        ap = orig + Point(10, -10)
        ic.Line(mr.BL() + Point(-4, 4), ap, self._HighlightColor)
        ic.PolyLine([ap + Point(0, -10), ap, ap + Point(10, 0)],
                    self._HighlightColor)

    def _MINameTool(self, ic):
        ic.SetFontSize(self._SafeSize - 10)
        ic.Text(self._SafeRect.center, 'N', self._GeneralColor)

    def _MINone(self, ic):
        # Create a null sign from a circle and diagonal.
        r = self._SafeRect.Shrink(24)
        ic.Circle(r.center, 24, self._GeneralColor)
        ic.Line(r.TR(), r.BL(), self._GeneralColor)

    def _MIOpenHelpPanel(self, ic):
        ic.SetFontSize(self._SafeSize - 10)
        ic.Text(self._SafeRect.center, '?', self._GeneralColor)

    def _MIOpenInfoPanel(self, ic):
        ic.SetLineWidth(8)
        ic.Circle(self._FullRect.center, 44, self._GeneralColor)
        ic.SetFontSize(60)
        ic.Text(self._SafeRect.center, 'i', self._GeneralColor)

    def _MIOpenSessionPanel(self, ic):
        self._AddFileIcon(ic)

    def _MIOpenSettingsPanel(self, ic):
        self._AddGear(ic, self._SafeRect.center, 24, 50, self._GeneralColor)

    def _MIPaste(self, ic):
        rr = self._SafeRect.Center(Point(60, 80)).Move(Point(0, 8))
        self._AddPaste(ic, rr)

    def _MIPasteInto(self, ic):
        ic.Circle(self._SafeRect.center, self._SafeSize / 2, self._GeneralColor)
        rr = self._SafeRect.Center(Point(54, 58)).Move(Point(0, 8))
        self._AddPaste(ic, rr)

    def _MIQuit(self, ic):
        r = self._SafeRect.Shrink(10).Left(70)
        ic.Rectangle(r, self._GeneralColor)
        # Space for arrow
        ar = Rect.FromCenterAndSize(r.CR(), Point(40, 28))
        ic.Rectangle(ar, self._Background, fill=True)
        # Arrow
        p = ar.CR()
        ic.Line(ar.CL(), p, self._GeneralColor)
        ic.PolyLine([p + Point(-10, -10), p, p + Point(-10, 10)],
                    self._GeneralColor)

    def _MIRadialLayout(self, ic):
        r = self._SafeRect.Shrink(12)
        # 3 cylinders
        rs = Point(20, 24)
        offset = Point(0, 20)
        r0 = Rect.FromCenterAndSize(r.CL()   + offset,   rs)
        r1 = Rect.FromCenterAndSize(r.center - offset, rs)
        r2 = Rect.FromCenterAndSize(r.CR()   + offset,   rs)
        ic.SetLineWidth(3)
        self._AddCylinder(ic, r0, self._LayoutColor)
        self._AddCylinder(ic, r1, self._LayoutColor)
        self._AddCylinder(ic, r2, self._LayoutColor)
        # Arcs connecting cylinders
        ic.SetLineWidth(5)
        rarc0 = Rect.FromCenterAndSize(Point(r1.min.x, r0.min.y),
                                       Point(r1.min.x - r0.center.x,
                                             r0.min.y - r1.center.y) * 2)
        rarc1 = Rect.FromCenterAndSize(Point(r1.max.x, r2.min.y),
                                       Point(r2.center.x - r1.max.x,
                                             r2.min.y - r1.center.y) * 2)
        ic.Arc(rarc0, 180, 270, self._LayoutColor)
        ic.Arc(rarc1, 270, 360, self._LayoutColor)

    def _MIRedo(self, ic):
        self._MIUndo(ic)
        ic.Mirror()

    def _MIRotationTool(self, ic):
        r = self._SafeRect.Shrink(10)
        ic.Arc(r, 160, 360 + 90, self._ToolColor)
        # Arrow
        ap = r.BC() + Point(0, -4)  # Center of line
        ic.PolyLine([ap + Point(12, 8), ap, ap + Point(12, -8)],
                    self._ToolColor)

    def _MIScaleTool(self, ic):
        sr = self._SafeRect.Shrink(8)
        # Little rectangle
        little_rect = sr.MoveLeftEdge(8).Bottom(50).Left(48)
        ic.Rectangle(little_rect, self._ToolColor)
        # Big rectangle.
        bpoints = [little_rect.TC(), Point(little_rect.center.x, sr.min.y),
                   sr.TR(), Point(sr.max.x, little_rect.center.y),
                   little_rect.CR()]
        ic.DashedPolyLine(bpoints, self._ToolColor)
        # Arrow
        ic.SetLineWidth(3)
        ap0 = little_rect.TR() + Point(2, -2)
        ap1 = sr.TR()          + Point(-8, 8)
        acolor = '#777777'
        ic.Line(ap0, ap1, acolor)
        ic.PolyLine([ap1 + Point(-8, 0), ap1, ap1 + Point(0, 8)], acolor)

    def _MISelectAll(self, ic):
        self._AddSelectRect(ic)
        # Checkmark
        ic.PolyLine([self._SelectRect.BL() + Point(24, -40),
                     self._SelectRect.BL() + Point(40, -24),
                     self._SelectRect.TR() + Point(-20, 20)], self._SelectColor)

    def _MISelectFirstChild(self, ic):
        self._AddSelectRect(ic)
        self._AddSelectGraph(ic)
        (parentc, lchildc, rchildc) = self._GetSelectGraphCenters()
        ap0 = Point(lchildc.x - 4, parentc.y - 10)
        ap1 = Point(lchildc.x - 4, parentc.y + 20)
        ic.Line(ap0, ap1, self._SelectArrowColor)
        ic.PolyLine([ap1 + Point(-6, -6), ap1, ap1 + Point(6, -6)],
                    self._SelectArrowColor)

    def _MISelectNextSibling(self, ic):
        self._AddSelectRect(ic)
        self._AddSelectGraph(ic)
        (parentc, lchildc, rchildc) = self._GetSelectGraphCenters()
        ap0 = lchildc
        ap1 = rchildc
        ic.Line(ap0, ap1, self._SelectArrowColor)
        ic.PolyLine([ap1 + Point(-6, -6), ap1, ap1 + Point(-6, 6)],
                    self._SelectArrowColor)

    def _MISelectNone(self, ic):
        self._AddSelectRect(ic)
        # X in center
        ic.SetFontSize(self._SelectRect.size.x - 30)
        ic.Text(self._SelectRect.center, 'X', self._SelectColor)

    def _MISelectParent(self, ic):
        self._AddSelectRect(ic)
        self._AddSelectGraph(ic)
        (parentc, lchildc, rchildc) = self._GetSelectGraphCenters()
        ap0 = Point(self._SelectRect.min.x + 8,  parentc.y)
        ap1 = Point(self._SelectRect.min.x + 26, parentc.y)
        ic.Line(ap0, ap1, self._SelectArrowColor)
        ic.PolyLine([ap1 + Point(-6, -6), ap1, ap1 + Point(-6, 6)],
                    self._SelectArrowColor)

    def _MISelectPreviousSibling(self, ic):
        self._AddSelectRect(ic)
        self._AddSelectGraph(ic)
        (parentc, lchildc, rchildc) = self._GetSelectGraphCenters()
        ap0 = rchildc
        ap1 = lchildc
        ic.Line(ap0, ap1, self._SelectArrowColor)
        ic.PolyLine([ap1 + Point(6, -6), ap1, ap1 + Point(6, 6)],
                    self._SelectArrowColor)

    def _MIShowAll(self, ic):
        self._AddEye(ic)
        ic.Blur(1)

    def _MISwitchToNextTool(self, ic):
        r = self._SafeRect.Shrink(10)
        self._AddWrench(ic, r.Left(40), self._ToolColor)
        # Arrow.
        ar = r.Right(40)
        ic.Rectangle(ar.Left(30).Center(Point(30, 8)), self._ToolColor,
                     fill=True)
        ap = r.CR()
        ic.Polygon([ap + Point(-10, -10), ap, ap + Point(-10, 10)],
                   self._ToolColor, fill=True)

    def _MISwitchToPreviousTool(self, ic):
        self._MISwitchToNextTool(ic)
        ic.Mirror()

    def _MIToggleAxisAligned(self, ic):
        origin = self._SafeRect.center + Point(-12, 12)
        ic.SetLineWidth(10)
        ic.Line(origin, origin + Point(-36,  40), '#6666ee')  # Z
        ic.Line(origin, origin + Point(  0, -70), '#66dd66')  # Y
        ic.Line(origin, origin + Point( 70,   0), '#ee6666')  # X
        pass

    def _MIToggleBuildVolume(self, ic):
        ic.Rectangle(self._SafeRect.Top(64).Center(Point(50, 64)),
                     self._GeneralColor)
        self._AddToggle(ic, self._GeneralColor)

    def _MIToggleEdgeTarget(self, ic):
        self._AddToggle(ic, self._LayoutColor)
        # Edge target.
        ic.SetLineWidth(4)
        end0 = self._ToggleRect.TC() + Point(0, -14)
        end1 = self._SafeRect.TC()   + Point(0, 4)
        ic.Rectangle(Rect.FromCenterAndSize(end0, Point(12, 18)),
                     self._LayoutColor, fill=True)
        ic.Line(end0, end1, self._LayoutColor)
        ic.Polygon([end1 + Point(-10, 14),
                    end1 + Point(0, -1),
                    end1 + Point(10, 14)],
                   self._LayoutColor, fill=True)

    def _MIToggleInspector(self, ic):
        # Magnifying glass
        r = self._SafeRect.Top(80).Right(80)
        ic.SetLineWidth(10)
        ic.Line(self._SafeRect.BL() + Point(6, -6), r.center,
                self._GeneralColor)
        ic.SetLineWidth(6)
        ic.Circle(r.center, r.size.x / 2, self._Background, fill=True)
        ic.Circle(r.center, r.size.x / 2, self._GeneralColor)
        # Object inside magnifying glass
        ic.SetLineWidth(3)
        self._AddCylinder(ic, r.Shrink(18), self._GeneralColor)

    def _MIToggleLeftRadialMenu(self, ic):
        self._AddToggleRadialMenu(ic, 'L')

    def _MITogglePointTarget(self, ic):
        self._AddToggle(ic, self._LayoutColor)
        # Point target.
        ic.SetLineWidth(4)
        end0 = self._ToggleRect.TC() + Point(0, -10)
        end1 = self._SafeRect.TC()   + Point(0, 4)
        ic.Circle(end0, 6, self._LayoutColor, fill=True)
        ic.Line(end0, end1, self._LayoutColor)
        ic.Polygon([end1 + Point(-10, 14),
                    end1 + Point(0, -1),
                    end1 + Point(10, 14)],
                   self._LayoutColor, fill=True)

    def _MIToggleRightRadialMenu(self, ic):
        self._AddToggleRadialMenu(ic, 'R')

    def _MIToggleShowEdges(self, ic):
        self._AddToggle(ic, self._GeneralColor)
        r = self._SafeRect.Top(60).Center(Point(60, 60))
        ic.SetLineWidth(2)
        self._AddCylinder(ic, r, self._GeneralColor)
        # Highlight edges
        p = r.BC() + Point(0, -3)
        ic.Line(p + Point(-16, -1), p + Point(-16, -43), self._HighlightColor)
        ic.Line(p, p + Point(0, -42), self._HighlightColor)
        ic.Line(p + Point(16, -1), p + Point(16, -43), self._HighlightColor)

    def _MIToggleSpecializedTool(self, ic):
        self._AddToggle(ic, self._GeneralColor)
        r = self._SafeRect.Top(80)
        self._AddWrench(ic, r, self._ToolColor)

    def _MITranslationTool(self, ic):
        rect = self._SafeRect.Shrink(6)
        l = rect.CL()
        r = rect.CR()
        t = rect.TC()
        b = rect.BC()
        # Lines
        ic.Line(l, r, self._ToolColor)
        ic.Line(t, b, self._ToolColor)
        a = 16
        # Left and right arrows.
        ic.PolyLine([l + Point( a, -a), l, l + Point( a, a)], self._ToolColor)
        ic.PolyLine([r + Point(-a, -a), r, r + Point(-a, a)], self._ToolColor)
        # Top and bottom arrows.
        ic.PolyLine([t + Point(-a,  a), t, t + Point(a,  a)], self._ToolColor)
        ic.PolyLine([b + Point(-a, -a), b, b + Point(a, -a)], self._ToolColor)

    def _MIUndo(self, ic):
        r = self._SafeRect.Top(90).Bottom(50)
        ic.SetLineWidth(14)
        ic.Arc(r, 230, 365, self._GeneralColor)
        # Arrow.
        ic.Polygon([Point(24, 46), Point(14, 80), Point(44, 79)],
                   self._GeneralColor, fill=True)

    # -------------------------------------------------------------------------
    # Helper functions.
    # -------------------------------------------------------------------------

    # Draws calipers to indicate precision.
    def _AddCalipers(self, ic, rect, color):
        ch = rect.size.y  # Full height of calipers
        cw = 50           # Full width of calipers
        hw = 20           # Width of handle
        jt = 12           # Thickness of a jaw
        js = 22           # Space between jaws

        cpoints = [
            Point(0, 0),                   # Top left.
            Point(cw - 20, 0),             # Angle point on top jaw.
            Point(cw, jt / 2),             # Top of end of top jaw.
            Point(cw, jt),                 # Bottom of end of top jaw.
            Point(hw, jt),                 # Top of space between jaws.
            Point(hw, jt + js),            # Bottom of space between jaws.
            Point(cw, jt + js),            # Top of end of bottom jaw.
            Point(cw, jt + js + jt / 2),   # Bottom of end of bottom jaw
            Point(cw - 20, jt + js + jt),  # Angle point on bottom jaw.
            Point(hw, jt + js + jt),       # Where bottom jaw meets handle.
            Point(hw, ch),                 # Bottom right point of handle.
            Point(0, ch),                  # Bottom left point of handle.
            Point(0, 0),                   # Top left.
        ]
        # The points look too much like an 'F', so reverse X and offset in X to
        # make room for more stuff below.
        offset = 10
        def _Reverse(p):
            return rect.TR() + Point(-p.x, p.y)
        ic.PolyLine([_Reverse(p + Point(10, 0)) for p in cpoints], color)
        # Add some things to make it look better.
        ic.Line(_Reverse(Point(offset + hw, jt)),
                _Reverse(Point(0, jt)), color)
        ic.Line(_Reverse(Point(0, jt + js)),
                _Reverse(Point(offset + hw, jt + js)), color)
        ic.SetLineWidth(2)
        ic.Circle(_Reverse(Point(cw - 2, jt + js / 2)), js / 2 - 4, color)

    # Adds the CSG rectangles and the given result rectangles.
    def _AddCSG(self, ic, result_rects):
        ic.Rectangle(self._CSGRect1, self._CSGColor1)
        ic.Rectangle(self._CSGRect2, self._CSGColor2)
        # Draw result on top.
        for rr in result_rects:
            ic.Rectangle(rr, self._CSGResColor, fill=True)

    def _AddCylinder(self, ic, rect, color):
        s = int(rect.size.y / 4)
        top = rect.Top(s)
        bot = rect.Bottom(s)
        ic.EllipseFromRect(top, color)
        ic.EllipseFromRect(bot, color)
        # Erase the top of the bottom ellipse.
        ic.Rectangle(bot.Top(int(s / 2) - 1), self._Background, fill=True)
        # Lines connecting the ellipses.
        ic.Line(top.CL(), bot.CL(), color)
        ic.Line(top.CR(), bot.CR(), color)

    def _AddEditPencil(self, ic, rect, color):
        hw = 4
        tip = Point(7, -7)
        tr = rect.TR() + Point(-hw, hw)
        bl = rect.BL() + Point(hw, -hw)
        pts = [tr + Point(-hw, -hw),
               tr + Point(hw, hw),
               bl + tip + Point(hw, hw),
               bl,
               bl + tip + Point(-hw, -hw),
               tr + Point(-hw, -hw)]
        # Erase behind lines first
        ic.Polygon(pts, self._Background, fill=True)
        # Then draw.
        ic.SetLineWidth(4)
        ic.Polygon(pts, color)
        ic.Line(pts[2], pts[4], color)

    def _AddEye(self, ic, size=100):
        r = self._SafeRect.Center(Point(size, size - 20))
        # Outside of eye.
        h = int(.8 * size) - 12  # Height of each piece of eye.
        a = 10  # Offset from full half-arc.
        tr = r.Bottom(h)
        br = r.Top(h)
        # This looks better as several thin arcs next to each other, rather
        # than one thick arc.
        ic.SetLineWidth(4)
        def _Arc(shrink):
            ic.Arc(tr.Shrink(shrink), 180 + a, 360 - a, self._GeneralColor)
            ic.Arc(br.Shrink(shrink),   0 + a, 180 - a, self._GeneralColor)
        for s in [0, 2, 4]:
            _Arc(s)
        # Iris
        rad = int(.35 * h) - 6   # Radius of iris
        ic.Circle(r.center, rad, self._GeneralColor, fill=True)

    def _AddFileIcon(self, ic):
        r = self._SafeRect.Center(Point(60, 80))
        ic.RoundedRectangle(r, 6, self._GeneralColor)
        # Remove top right corner
        r2 = Rect.FromMinMax(r.TC(), r.TR() + Point(0, 30))
        ic.Rectangle(r2.MoveTopEdge(-4).MoveRightEdge(10),
                     self._Background, fill=True)
        # Edge of flap, fudging a little
        fudge = Point(1, 1)
        ic.Line(r2.TL() - fudge, r2.BR() + fudge, self._GeneralColor)
        # Rest of flap
        ic.SetLineWidth(4)
        fr = r2.Bottom(20).Left(20)
        ic.Arc(fr, 90, 180, self._GeneralColor)
        ic.Line(r2.TL(), fr.CL(), self._GeneralColor)
        ic.Line(r2.BR(), fr.BC(), self._GeneralColor)

    def _AddGear(self, ic, center, inner_radius, outer_radius, color):
        def _Rot(p, angle_in_degrees):
            angle = radians(angle_in_degrees)
            return Point(round(cos(angle) * p.x - sin(angle) * p.y),
                         round(sin(angle) * p.x + cos(angle) * p.y))

        # Inner circle
        ic.Circle(center, inner_radius, self._GeneralColor)
        # Outer gear shape
        w1 = round(.4  * outer_radius)
        w2 = round(.32 * outer_radius)
        h  = round(.24 * outer_radius)
        tooth_pts = [Point(-w1 / 2,  h), Point(-w2 / 2,  0),
                     Point( w2 / 2,  0), Point( w1 / 2,  h)]
        pts = []
        for i in range(8):
            oangle = i * 360.0 / 8
            offset = Point(round(outer_radius * cos(radians(oangle))),
                           round(outer_radius * sin(radians(oangle))))
            pts += [center + offset + _Rot(tp, oangle + 90) for tp in tooth_pts]
        pts.append(pts[0])  # Close it
        ic.PolyLine(pts, self._GeneralColor)

    # Adds a list of 3 symbolic names and an arrow indicating movement of the
    # middle name up or down.
    def _AddMoveName(self, ic, up_or_down):
        r = self._SafeRect.Shrink(10)
        listr = r.Left(60).Center(Point(60, 70))
        # Name text
        textr1 = listr.Top(20)
        textr2 = listr.Center(Point(60, 20))
        textr3 = listr.Bottom(20)
        text_color = '#aaaac3'
        ic.Text(textr1.center, 'XxxXxx', text_color)
        ic.Text(textr2.center, 'XxXxxx', text_color)
        ic.Text(textr3.center, 'XxxxXx', text_color)
        # Move highlighting box and arrow
        ic.SetLineWidth(1)
        ic.Rectangle(textr2, self._HighlightColor)
        arcr = r.Right(50).Center(Point(30, 70))
        if up_or_down == 'up':
            arcr = arcr.Top(35)
            ap0 = arcr.TC() + Point(0, 2)
        else:
            arcr = arcr.Bottom(35).MoveBottomEdge(6)
            ap0 = arcr.BC() + Point(0, -2)
        ic.SetLineWidth(3)
        ic.Arc(arcr, 270, 90, self._HighlightColor)
        ap1 = ap0 + Point(-6, 0)
        ic.Line(ap0, ap1, self._HighlightColor)
        ic.PolyLine([ap1 + Point(6, -6), ap1, ap1 + Point(6, 6)],
                    self._HighlightColor)

    # Adds a radial menu icon with the given letter.
    def _AddToggleRadialMenu(self, ic, letter):
        self._AddToggle(ic, self._GeneralColor)
        r = self._SafeRect.Top(64).Center(Point(50, 64))
        ic.SetLineWidth(8)
        ic.Circle(r.center, 32, self._MenuColor)
        ic.SetFontSize(50)
        ic.Text(r.center, letter, self._HighlightColor)

    # Adds the paste icon using the given rectangle for the clipboard.This
    # allows it to be used for both paste and paste-into icons.
    def _AddPaste(self, ic, clipboard_rect):
        rr = clipboard_rect
        cr = rr.Center(Point(30, 20)).Move(Point(0, -rr.size.y / 2))
        # Clipboard.
        ic.RoundedRectangle(rr, 6, self._GeneralColor)
        # Clipboard ring.
        ic.SetLineWidth(6)
        ic.Circle(cr.TC() - Point(0, 4), 10, self._GeneralColor)
        # Clipboard clip.
        ic.Rectangle(cr, self._Background, fill=True)
        ic.Rectangle(cr, self._GeneralColor)

    # Adds a plus for creation icons.
    def _AddPlus(self, ic):
        plr = self._CreationPlusRect
        ic.Line(plr.CL(), plr.CR(), self._CreateColor)
        ic.Line(plr.TC(), plr.BC(), self._CreateColor)

    def _AddSelectGraph(self, ic):
        # Circle centers
        rad = 14
        (parentc, lchildc, rchildc) = self._GetSelectGraphCenters()
        ic.SetLineWidth(3)
        # Draw connecting lines first.
        ic.PolyLine([lchildc, parentc, rchildc], self._SelectColor)
        # Circles: erase lines and draw
        ic.Circle(parentc, rad, self._Background, fill=True)
        ic.Circle(lchildc, rad, self._Background, fill=True)
        ic.Circle(rchildc, rad, self._Background, fill=True)
        ic.Circle(parentc, rad, self._SelectColor)
        ic.Circle(lchildc, rad, self._SelectColor)
        ic.Circle(rchildc, rad, self._SelectColor)

    def _AddSelectRect(self, ic, color=None):
        ic.RoundedRectangle(self._SelectRect, 8,
                            color if color else self._SelectColor)

    def _AddToggle(self, ic, color):
        # Capsule shape.
        r  = self._ToggleRect
        lr = r.Left(r.size.y)
        rr = r.Right(r.size.y)
        ic.Arc(lr,  90, 270, color, fill=True)
        ic.Arc(rr, 270,  90, color, fill=True)
        ic.Rectangle(Rect.FromMinMax(lr.TC(), rr.BC()), color, fill=True)
        # Clear circle in it.
        ic.Circle(rr.center, (rr.size.x / 2) - 2, self._Background, fill=True)

    def _AddTorus(self, ic, rect, color):
        outer = rect
        inner = rect.Center(outer.size / 2)
        ic.EllipseFromRect(outer, color)
        ic.EllipseFromRect(inner, color)

    def _AddWrench(self, ic, rect, color):
        # Height and width of handle
        hwidth  = 10
        hheight = rect.size.y / 2
        # Radius of round part of jaws
        radius = 14
        # Main part of handle
        hr = rect.Center(Point(hwidth, hheight)).Move(Point(0, radius / 2))
        ic.Rectangle(hr, color, fill=True)
        # Semicircle at bottom
        ic.Arc(Rect.FromCenterAndSize(hr.BC(), Point(hwidth, hwidth)), 0, 180,
               color, fill=True)
        # Circle for jaws
        jc = hr.TC() + Point(0, -radius / 2)
        ic.Circle(jc, radius, color, fill=True) # Cut out a rectangle
        ic.Rectangle(Rect.FromCenterAndSize(jc + Point(0, -10),
                                            Point(8, 20)),
                     self._Background, fill=True)

    def _GetSelectGraphCenters(self):
        offset = 24
        # Circle centers
        parentc = self._SelectRect.TC() + Point(0,        offset)
        lchildc = self._SelectRect.BL() + Point( offset, -offset)
        rchildc = self._SelectRect.BR() + Point(-offset, -offset)
        return (parentc, lchildc, rchildc)
