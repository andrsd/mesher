import vtk


def buildPointArray2D(points):
    n_points = len(points)
    point_array = vtk.vtkPoints()
    point_array.Allocate(n_points)
    for i, pt in enumerate(points):
        point_array.InsertPoint(i, [pt[0], pt[1], 0.])
    return point_array


def buildPointArray3D(points):
    n_points = len(points)
    point_array = vtk.vtkPoints()
    point_array.Allocate(n_points)
    for i, pt in enumerate(points):
        point_array.InsertPoint(i, [pt[0], pt[1], pt[2]])
    return point_array


def buildCellArrayVertex(vertices):
    cell_array = vtk.vtkCellArray()
    cell_array.Allocate(len(vertices))
    for i in range(len(vertices)):
        elem = vtk.vtkVertex()
        elem.GetPointIds().SetId(0, i)
        cell_array.InsertNextCell(elem)
    return cell_array


def buildCellArrayLine(segments):
    n_segments = len(segments)
    cell_array = vtk.vtkCellArray()
    cell_array.Allocate(n_segments)
    for seg in list(segments):
        elem = vtk.vtkLine()
        elem.GetPointIds().SetId(0, int(seg[0]))
        elem.GetPointIds().SetId(1, int(seg[1]))
        cell_array.InsertNextCell(elem)
    return cell_array


def buildCellArrayTriangle(triangles):
    n_triangles = len(triangles)
    cell_array = vtk.vtkCellArray()
    cell_array.Allocate(n_triangles)
    for tri in list(triangles):
        elem = vtk.vtkTriangle()
        elem.GetPointIds().SetId(0, int(tri[0]))
        elem.GetPointIds().SetId(1, int(tri[1]))
        elem.GetPointIds().SetId(2, int(tri[2]))
        cell_array.InsertNextCell(elem)
    return cell_array


def buildCellArrayTetra(tetras):
    n_elems = len(tetras)
    cell_array = vtk.vtkCellArray()
    cell_array.Allocate(n_elems)
    for e in list(tetras):
        elem = vtk.vtkTetra()
        elem.GetPointIds().SetId(0, int(e[0]))
        elem.GetPointIds().SetId(1, int(e[1]))
        elem.GetPointIds().SetId(2, int(e[2]))
        elem.GetPointIds().SetId(3, int(e[3]))
        cell_array.InsertNextCell(elem)
    return cell_array


def buildCellArrayPolygon(facets):
    cell_array = vtk.vtkCellArray()
    for facet in list(facets):
        for poly in list(facet.polygons):
            elem = vtk.vtkPolygon()
            elem.GetPointIds().SetNumberOfIds(len(poly.vertices))
            for i, vertex_id in enumerate(poly.vertices):
                elem.GetPointIds().SetId(i, int(vertex_id))
            cell_array.InsertNextCell(elem)
    return cell_array


def vertices2DToUnstructuredGrid(points):
    pt_arr = buildPointArray2D(points)
    cell_arr = buildCellArrayVertex(points)
    ugrid = vtk.vtkUnstructuredGrid()
    ugrid.SetPoints(pt_arr)
    ugrid.SetCells(vtk.VTK_VERTEX, cell_arr)
    return ugrid


def vertices3DToUnstructuredGrid(points):
    pt_arr = buildPointArray3D(points)
    cell_arr = buildCellArrayVertex(points)
    ugrid = vtk.vtkUnstructuredGrid()
    ugrid.SetPoints(pt_arr)
    ugrid.SetCells(vtk.VTK_VERTEX, cell_arr)
    return ugrid
