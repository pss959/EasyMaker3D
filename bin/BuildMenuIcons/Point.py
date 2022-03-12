# -----------------------------------------------------------------------------
# 2D Point class.
# -----------------------------------------------------------------------------

class Point(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    @classmethod
    def Copy(cls, pt):
        return cls(pt.x, pt.y)

    def __getitem__(self, index):
        assert(index == 0 or index == 1)
        return x if index == 0 else y

    def __add__(self, p):
        return Point(self.x + p.x, self.y + p.y)

    def __sub__(self, p):
        return Point(self.x - p.x, self.y - p.y)

    def __mul__(self, s):
        return Point(self.x * s, self.y * s)

    def __truediv__(self, q):
        return Point(int(self.x / q), int(self.y / q))

    def __str__(self):
        return '(' + str(self.x) + ',' + str(self.y) + ')'

    def Tuple(self):
        return (self.x, self.y)
