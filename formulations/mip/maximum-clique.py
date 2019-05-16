#!/usr/bin/env python3

import click
import gurobipy

from graph import read_dimacs


def create_model(vertices, edges):
    model = gurobipy.Model()
    xvars = [model.addVar(vtype='B') for i in range(vertices)]
    model.update()
    for i in range(vertices):
        for j in range(i + 1, vertices):
            if (i, j) not in edges:
                model.addConstr(xvars[i] + xvars[j] <= 1)
    model.setObjective(gurobipy.quicksum(xvars), gurobipy.GRB.MAXIMIZE)
    model.update()
    return model


if __name__ == '__main__':

    @click.command()
    @click.argument("input-file", type=click.File('r'))
    def run(input_file):
        vertices, edges = read_dimacs(input_file)
        print(f"Vertices: {vertices}")
        print(f"Edges: {len(edges)}")
        model = create_model(vertices, edges)
        model.optimize()

    run()
