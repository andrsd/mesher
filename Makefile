SRC=mesher

all:
	@echo ""

init:
	@pip install -e .
	@pip install -r requirements/test.txt

syntax-check check-syntax:
	@flake8 $(SRC) mesher examples tests

test:
	@PYTEST_QT_API=pyqt5 pytest .

coverage:
	@PYTEST_QT_API=pyqt5 coverage run --source=$(SRC) -m pytest -v -s
	@coverage html
