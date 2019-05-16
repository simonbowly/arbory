#!/usr/bin/env python3

import click
import gurobipy

from graph import read_dimacs


def create_model(vertices, edges, maxcolors):
    model = gurobipy.Model()
    x = [
        [model.addVar(vtype='B') for k in range(maxcolors)]
        for i in range(vertices)]
    y = [model.addVar(vtype='B') for k in range(maxcolors)]
    model.update()
    for i in range(vertices):
        model.addConstr(gurobipy.quicksum(x[i]) == 1)
    for k in range(maxcolors):
        for i in range(vertices):
            model.addConstr(x[i][k] <= y[k])
        for i, j in edges:
            model.addConstr(x[i][k] + x[j][k] <= 1)
    model.setObjective(gurobipy.quicksum(y), gurobipy.GRB.MINIMIZE)
    model.update()
    return model


if __name__ == '__main__':

    @click.command()
    @click.argument("input-file", type=click.File('r'))
    @click.option("--maxcolors", type=int)
    def run(input_file, maxcolors):
        vertices, edges = read_dimacs(input_file)
        print(f"Vertices: {vertices}")
        print(f"Edges: {len(edges)}")
        model = create_model(vertices, edges, maxcolors)
        model.optimize()

    run()
