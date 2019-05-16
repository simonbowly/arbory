#!/usr/bin/env python3

import pathlib
import click

base = pathlib.Path(__file__).parent.resolve()

@click.command()
@click.argument("n", type=int)
def run(n):
    with open(base.joinpath("v100-e1902.col")) as infile:
        lines = list(infile)
    edges = [
        f"e {i} {j}"
        for _, i, j in (
            line.split()
            for line in lines
            if line.startswith('e'))
        if int(i) <= n and int(j) <= n
    ]
    content = f"p edge {n} {len(edges)}\n" + "\n".join(edges)
    with open(base.joinpath(f"v{n:03d}-e{len(edges)}.col"), 'w') as outfile:
        outfile.write(content)

run()
