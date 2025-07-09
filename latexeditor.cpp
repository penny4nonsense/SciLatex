#include "latexeditor.h"
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>

LatexEditor::LatexEditor(QWidget *parent) : QMainWindow(parent) {
    // Initialize components
    latexCodeEditor = new QTextEdit;
    compiledOutputViewer = new QTextEdit;

    // Set placeholder text to make panes immediately visible
    latexCodeEditor->setPlaceholderText("Enter LaTeX code here...");
    compiledOutputViewer->setPlaceholderText("Compiled output will be shown here...");

    // Ensure each QTextEdit has a minimum size
    latexCodeEditor->setMinimumSize(200, 300);
    compiledOutputViewer->setMinimumSize(200, 300);

    // Initialize the splitter for side-by-side view
    splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(latexCodeEditor);
    splitter->addWidget(compiledOutputViewer);

    // Initialize the stacked widget with placeholders for individual views
    codeViewContainer = new QWidget;
    QVBoxLayout *codeLayout = new QVBoxLayout(codeViewContainer);
    codeLayout->addWidget(latexCodeEditor);

    compiledViewContainer = new QWidget;
    QVBoxLayout *compiledLayout = new QVBoxLayout(compiledViewContainer);
    compiledLayout->addWidget(compiledOutputViewer);

    // Use dynamic widget switching instead of stacked widget
    setCentralWidget(codeViewContainer);  // Start with code view as default

    setupMenu();  // Set up the menu for view selection
}

void LatexEditor::setupMenu() {
    // Create actions for each view
    QAction *codeViewAction = new QAction("Code View", this);
    QAction *compiledViewAction = new QAction("Compiled View", this);
    QAction *sideBySideViewAction = new QAction("Side-by-Side View", this);

    // Connect actions to switch views by replacing central widget
    connect(codeViewAction, &QAction::triggered, this, [=]() {
        setCentralWidget(codeViewContainer); // Switch to code view
    });
    connect(compiledViewAction, &QAction::triggered, this, [=]() {
        setCentralWidget(compiledViewContainer); // Switch to compiled view
    });
    connect(sideBySideViewAction, &QAction::triggered, this, [=]() {
        setCentralWidget(splitter);  // Switch to side-by-side view
        splitter->setVisible(true);  // Ensure splitter visibility
        splitter->update();          // Update splitter layout
    });

    // Add actions to the menu bar
    QMenu *viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(codeViewAction);
    viewMenu->addAction(compiledViewAction);
    viewMenu->addAction(sideBySideViewAction);
}
