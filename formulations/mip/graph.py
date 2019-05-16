
def read_dimacs(infile):
    lines = list(infile)
    _, _, vertices, nedges = next(
        line.split()
        for line in lines
        if line.startswith('p')
        )
    vertices = int(vertices)
    nedges = int(nedges)
    edges = {
        (int(i) - 1, int(j) - 1)
        for _, i, j in (
            line.split()
            for line in lines
            if line.startswith('e')
            )}
    assert nedges == len(edges)
    return vertices, edges
