#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

# -----------------------------------------------------------------------------
# Rect class that makes some things easier in ImageDraw.
#
# NOTE that y increases from top to bottom.
# -----------------------------------------------------------------------------

from BuildMenuIcons.Point import Point

class Rect(object):

    # -------------------------------------------------------------------------
    # Initialization from center Point and size Point.
    # -------------------------------------------------------------------------

    def __init__(self, center, size):
        self.center = center
        self.size   = size
        self.min    = center - size / 2
        self.max    = center + size / 2

    # -------------------------------------------------------------------------
    # Factory functions.
    # -------------------------------------------------------------------------

    @classmethod
    def FromSize(cls, size):
        return cls(size / 2, size)

    @classmethod
    def FromCenterAndSize(cls, center, size):
        return cls(center, size)

    @classmethod
    def FromMinMax(cls, minpt, maxpt):
        return cls((minpt + maxpt) / 2, maxpt - minpt)

    # -------------------------------------------------------------------------
    # Conversion.
    # -------------------------------------------------------------------------

    def ToList(self):
        'Returns the min and max points as a list of 2 tuples.'
        return [self.min.Tuple(), self.max.Tuple()]

    def __str__(self):
        return 'Rect[min=' + str(self.min) + ', max=' + str(self.max) + ']'

    # -------------------------------------------------------------------------
    # Point access: Top/Center/Bottom + Left/Center/Right.
    # -------------------------------------------------------------------------

    def TL(self):
        return self.min

    def TC(self):
        return Point(self.center.x, self.min.y)

    def TR(self):
        return Point(self.max.x, self.min.y)

    def CL(self):
        return Point(self.min.x, self.center.y)

    def CR(self):
        return Point(self.max.x, self.center.y)

    def BL(self):
        return Point(self.min.x, self.max.y)

    def BC(self):
        return Point(self.center.x, self.max.y)

    def BR(self):
        return self.max

    # -------------------------------------------------------------------------
    # Sub-rectangle access. Remember that Y increases downwards.
    # -------------------------------------------------------------------------

    def Top(self, height):
        return Rect.FromMinMax(self.TL(), self.TR() + Point(0, height))

    def Bottom(self, height):
        return Rect.FromMinMax(self.BL() - Point(0, height), self.BR())

    def Left(self, width):
        return Rect.FromMinMax(self.TL(), self.BL() + Point(width, 0))

    def Right(self, width):
        return Rect.FromMinMax(self.TR() - Point(width, 0), self.BR())

    def Center(self, size):
        return Rect.FromCenterAndSize(self.center, size)

    def Shrink(self, delta_per_side):
        return self.Center(self.size - Point(1, 1) * (2 * delta_per_side))

    def Overlap(self, r):
        # There must be some overlap in both dimensions.
        assert(r.min.x <= self.max.x and r.max.x >= self.min.x)
        assert(r.min.y <= self.max.y and r.max.y >= self.min.y)
        return Rect.FromMinMax(Point(max(r.min.x, self.min.x),
                                     max(r.min.y, self.min.y)),
                               Point(min(r.max.x, self.max.x),
                                     min(r.max.y, self.max.y)))

    # -------------------------------------------------------------------------
    # New Rect formed by moving.
    # -------------------------------------------------------------------------

    def Move(self, delta):
        return Rect.FromCenterAndSize(self.center + delta, self.size)

    # -------------------------------------------------------------------------
    # New Rect formed by moving one side, keeping other things constant.
    # -------------------------------------------------------------------------

    def MoveLeftEdge(self, delta):
        return Rect.FromMinMax(self.TL() + Point(delta, 0), self.BR())

    def MoveRightEdge(self, delta):
        return Rect.FromMinMax(self.TL(), self.BR() + Point(delta, 0))

    def MoveTopEdge(self, delta):
        return Rect.FromMinMax(self.TL() + Point(0, delta), self.BR())

    def MoveBottomEdge(self, delta):
        return Rect.FromMinMax(self.TL(), self.BR() + Point(0, delta))
