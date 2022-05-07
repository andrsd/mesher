import vtk


class Selection(object):

    def __init__(self, data):
        """
        @param data vtkDataObject that will be "filtered" for selection
        """
        self.cells = {}

        self._selection = vtk.vtkSelection()

        self._extract_selection = vtk.vtkExtractSelection()
        self._extract_selection.SetInputData(0, data)
        self._extract_selection.SetInputData(1, self._selection)

        self._selected = vtk.vtkUnstructuredGrid()

        self._mapper = vtk.vtkDataSetMapper()
        self._mapper.SetInputData(self._selected)

        self._actor = vtk.vtkActor()
        self._actor.SetMapper(self._mapper)

    def getActor(self):
        return self._actor

    def get(self):
        return self._selected

    def clear(self):
        self.cells = {}
        self._selected.Initialize()
        self._mapper.SetInputData(self._selected)

    def deselectAll(self):
        self.cells = {}
        self._selected.Initialize()
        self._selection.RemoveAllNodes()
        self._mapper.Update()

    def selectCell(self, cell_id):
        # TODO: this method could use a better name
        ids = vtk.vtkIdTypeArray()
        ids.SetNumberOfComponents(1)
        ids.InsertNextValue(cell_id)

        selection_node = vtk.vtkSelectionNode()
        selection_node.SetFieldType(vtk.vtkSelectionNode.CELL)
        selection_node.SetContentType(vtk.vtkSelectionNode.INDICES)
        selection_node.SetSelectionList(ids)

        self._selection.RemoveAllNodes()
        self._selection.AddNode(selection_node)
        self.cells[cell_id] = selection_node

        self._extract_selection.Update()
        self._selected.ShallowCopy(self._extract_selection.GetOutput())

    def hasCell(self, cell_id):
        return cell_id in self.cells

    def addCell(self, cell_id):
        ids = vtk.vtkIdTypeArray()
        ids.SetNumberOfComponents(1)
        ids.InsertNextValue(cell_id)

        selection_node = vtk.vtkSelectionNode()
        selection_node.SetFieldType(vtk.vtkSelectionNode.CELL)
        selection_node.SetContentType(vtk.vtkSelectionNode.INDICES)
        selection_node.SetSelectionList(ids)
        self._selection.AddNode(selection_node)
        self.cells[cell_id] = selection_node

        self._extract_selection.Update()
        self._selected.ShallowCopy(self._extract_selection.GetOutput())

    def removeCell(self, cell_id):
        node = self.cells.pop(cell_id, None)
        self._selection.RemoveNode(node)
        self._extract_selection.Update()
        self._selected.ShallowCopy(self._extract_selection.GetOutput())
