class Graph:
    """Union-find minimal pour le contrôle de connectivité."""
    def __init__(self):
        self.p = {}

    def add(self, a):
        self.p.setdefault(a, a)

    def find(self, a):
        while self.p[a] != a:
            self.p[a] = self.p[self.p[a]]
            a = self.p[a]
        return a

    def link(self, a, b):
        ra, rb = self.find(a), self.find(b)
        if ra != rb:
            self.p[ra] = rb

    def components(self, ids):
        groups = {}
        for i in ids:
            groups.setdefault(self.find(i), []).append(i)
        return list(groups.values())
