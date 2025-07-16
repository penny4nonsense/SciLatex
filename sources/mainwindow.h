#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QtPdf/QPdfDocument>
#include <QtPdfWidgets/QPdfView>
#include <QSet>
#include <QMap>
#include <QProcess>
#include <QPushButton>
#include <QLineEdit>
#include <stack>
#include <QPair>
#include <QTextCursor>
#include <QStack>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

// Track placeholders dynamically
struct Placeholder {
    QTextCursor cursor; // Cursor pointing to the placeholder
    bool visited;       // Whether the placeholder has been visited
    int i;              // Index of the placeholder batch
    int j;              // Placeholder number
};


protected:
    // Override eventFilter to capture events in latexEditor
    bool eventFilter(QObject *obj, QEvent *event) override;

    // Override keyPressEvent for custom key handling in MainWindow
    void keyPressEvent(QKeyEvent *event) override;

    // Override keyReleaseEvent for custom key handling in MainWindow
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    // File menu actions
    void newFile();
    void openFile();
    void closeFile();
    void saveFile();
    void saveFileAs();
    void compile();
    void importFile();
    void exportFile();
    void preview();
    void printFile();
    void preferences();
    void recentFiles();
    void exitApp();

    // Edit menu actions
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void deleteContent();
    void selectAll();
    void findText();
    void replaceText();
    void spellCheck();

    // Insert menu actions
    void insertMathText();
    void insertFraction();
    void insertRadical();
    void insertSuperscript();
    void insertSubscript();
    void insertDisplay();
    void insertOperator();
    void insertBracket();
    void insertMatrix();
    void insertMathName();
    void insertBinomial();
    void insertLabel();
    void insertDecoration();
    void insertFontStyle();
    void insertSpacing();
    void insertTable();
    void insertNote();
    void insertFormula();
    void insertHyperlink();
    void insertMarker();
    void insertHtmlField();

    // View menu actions
    void setFontSize(int points);
    void customFontSize();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void refreshView();

    // Compute menu actions
    void evaluate();
    void evaluateNumerically();
    void simplify();
    void combine();
    void factor();
    void expand();
    void rewrite();
    void checkEquality();
    void solve();
    void plot2D();
    void plot3D();
    void computeSettings();

    // New slots for the search/replace actions:
    void onFindPrevious();
    void onFindNext();
    void onReplacePrevious();
    void onReplaceNext();
    void onReplaceAll();


private:
    QSplitter *splitter;
    QPlainTextEdit *latexEditor;
    QPdfDocument *pdfDocument;
    QPdfView *pdfView;
    QSet<int> activeKeys;        // Track currently pressed keys
    QSet<Qt::KeyboardModifier> activeModifiers; // Track active modifiers
    QString keyChain;            // Track the sequence of keys pressed
    QMap<QString, QString> keychainActions; // Map keychains to LaTeX commands
    void processKeychain();                 // Process the finalized keychain
    bool isGreekMode;                        // Tracks if Greek mode is active
    QMap<QString, QString> greekLetterMap;   // Maps letters to Greek LaTeX commands
    void handleGreekMode(QString key);       // Handles Greek letter mapping
    bool isMatrixMode;
    void handleMatrixMode(QString key);
    void createMenus();
    void createActions();
    QMenu *createSubmenu(QMenu *parentMenu, const QString &title, const QStringList &actions, void (MainWindow::*handler)(QString));

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *insertMenu;
    QMenu *viewMenu;
    QMenu *computeMenu;

    void loadPreferences();
    QString currentFile;  // Path to the current file
    bool isModified;      // Tracks if the current file has unsaved changes

    // Utility methods
    bool checkUnsavedChanges();
    void writeToFile(const QString &fileName);

    QString latexTemplate; // Stores the default LaTeX template
    void loadDefaultTemplate();

    QString latexInterpreterPath;
    QString matlabPath;
    QList<QString> recentFilesList;

    QVector<QString> undoStack;     // Stack for undo operations
    QVector<QString> redoStack;     // Stack for redo operations
    void saveState();               // Save the current state to the undo stack
    void applyState(QVector<QString>& sourceStack, QVector<QString>& targetStack, const QString& action);
    void loadPDF(const QString& pdfFilePath);

    QProcess *pdflatexProcess;  // Background process for pdflatex
    void reloadPDF();           // Function to reload PDF

    // Add these new pointers for the find toolbar
    QWidget *findWidget;
    QLineEdit *findLineEdit;

    // Add these new pointers for the replace toolbar
    QWidget *replaceWidget;
    QLineEdit *replaceFindLineEdit;
    QLineEdit *replaceLineEdit;

    // If you have buttons that need to be accessed outside the constructor,
    // you can declare them here too, though it's often okay to keep them local
    // as long as you connect signals and slots in the constructor.
    // For clarity, you might store them here if you need to show/hide them dynamically.
    QPushButton *findPrevButton;
    QPushButton *findNextButton;
    QPushButton *closeFindButton;

    QPushButton *rFindPrevButton;
    QPushButton *rFindNextButton;
    QPushButton *closeReplaceButton;
    QPushButton *replacePrevButton;
    QPushButton *replaceNextButton;
    QPushButton *replaceAllButton;

    QDockWidget *searchDock;
    void processLatexOperation(const std::string& operation);
    void openRecentFile(const QString &filePath);
    void clearRecentFiles();
    QStringList loadRecentFiles();
    void updateRecentFiles(const QString &filePath);
    void toggleMathTextMode();

    // Add placeholder management
    std::stack<QPair<int, int>> placeholderStack; // Stores position and length of each placeholder
    void processAndInsertText(const QString &stuff);
    void navigateToNextPlaceholder();
    void navigateToPreviousPlaceholder();

    void onContentsChange(int position, int charsRemoved, int charsAdded);
    QStack<Placeholder> forwardStack; // Placeholders to visit in the forward direction
    QStack<Placeholder> backwardStack; // Placeholders for reverse navigation
};

#endif // MAINWINDOW_H
