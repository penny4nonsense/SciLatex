#include "mainwindow.h"
#include "keyutils.h"
#include "navigation.h"
#include "matrixmode.h"
#include "mathnamedialog.h"
#include "labeldialog.h"
#include "decoratordialog.h"
#include "bracketchooserdialog.h"
#include "operatordialog.h"
#include "preferencesdialog.h"
#include "previewwindow.h"
#include "preferencesmanager.h"
#include "spacingdialog.h"
#include "recentfilesdialog.h"
#include "fontdialog.h"
#include "matlabutils.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDebug>
#include <QStatusBar>
#include <QShortcut>
#include <QMenu>
#include <QMenuBar>
#include <QActionGroup>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>
#include <QtPdf/QPdfDocument>
#include <QtPdf/QPdfDocumentRenderOptions>
#include <QtPdfWidgets/QPdfView>
#include <QProcess>
#include <QFileInfo>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QDockWidget>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <QTextBlock>

QString currentFile;
bool isModified = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    splitter(new QSplitter(this)),
    latexEditor(new QPlainTextEdit(this)),
    pdfDocument(new QPdfDocument(this)), // Initialize QPdfDocument
    pdfView(new QPdfView(this)),    // Initialize QPdfView
    pdflatexProcess(new QProcess(this)),
    keyChain(""),
    isGreekMode(false),
    isMatrixMode(false) {

    loadDefaultTemplate(); // Load the default template

    // Configure the LaTeX editor (left pane)
    latexEditor->setWordWrapMode(QTextOption::NoWrap);
    latexEditor->setPlainText(latexTemplate);

    // Disable default Select All shortcut
    latexEditor->setTextInteractionFlags(Qt::TextEditorInteraction);
    latexEditor->installEventFilter(this);
    latexEditor->setUndoRedoEnabled(false);

    // Add the LaTeX editor to the splitter
    splitter->addWidget(latexEditor);

    pdfView->setDocument(pdfDocument); // Link QPdfDocument to QPdfView
    pdfView->setPageMode(QPdfView::PageMode::MultiPage);  // Example setting for QPdfView
    pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    pdfView->setZoomFactor(1.0);

    splitter->addWidget(pdfView);  // Add MuPDFView to the splitter

    connect(pdflatexProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::reloadPDF);

    // Connect to capture standard error output
    connect(pdflatexProcess, &QProcess::readyReadStandardError, [this]() {
        QByteArray errorOutput = pdflatexProcess->readAllStandardError();
        qDebug() << "[PDFLaTeX Error]" << QString(errorOutput);
    });

    // Add the splitter to the central widget
    setCentralWidget(splitter);

    // Add a status bar
    statusBar()->showMessage("Ready");

    // Create the find toolbar widget
    findWidget = new QWidget(this);
    QHBoxLayout *findLayout = new QHBoxLayout(findWidget);
    findLineEdit = new QLineEdit(findWidget);
    QPushButton *findPrevButton = new QPushButton("Find Previous", findWidget);
    QPushButton *findNextButton = new QPushButton("Find Next", findWidget);
    findLayout->addWidget(findLineEdit);
    findLayout->addWidget(findPrevButton);
    findLayout->addWidget(findNextButton);
    findWidget->setLayout(findLayout);

    // Create the replace toolbar widget
    replaceWidget = new QWidget(this);
    QVBoxLayout *replaceMainLayout = new QVBoxLayout(replaceWidget);

    // Top row (like find)
    QWidget *replaceTopRow = new QWidget(replaceWidget);
    QHBoxLayout *replaceTopLayout = new QHBoxLayout(replaceTopRow);
    replaceFindLineEdit = new QLineEdit(replaceTopRow);
    QPushButton *rFindPrevButton = new QPushButton("Find Previous", replaceTopRow);
    QPushButton *rFindNextButton = new QPushButton("Find Next", replaceTopRow);
    replaceTopLayout->addWidget(replaceFindLineEdit);
    replaceTopLayout->addWidget(rFindPrevButton);
    replaceTopLayout->addWidget(rFindNextButton);
    replaceTopRow->setLayout(replaceTopLayout);

    // Bottom row (Replace line)
    QWidget *replaceBottomRow = new QWidget(replaceWidget);
    QHBoxLayout *replaceBottomLayout = new QHBoxLayout(replaceBottomRow);
    replaceLineEdit = new QLineEdit(replaceBottomRow);
    QPushButton *replacePrevButton = new QPushButton("Replace Previous", replaceBottomRow);
    QPushButton *replaceNextButton = new QPushButton("Replace Next", replaceBottomRow);
    QPushButton *replaceAllButton = new QPushButton("Replace All", replaceBottomRow);
    replaceBottomLayout->addWidget(replaceLineEdit);
    replaceBottomLayout->addWidget(replacePrevButton);
    replaceBottomLayout->addWidget(replaceNextButton);
    replaceBottomLayout->addWidget(replaceAllButton);
    replaceBottomRow->setLayout(replaceBottomLayout);

    replaceMainLayout->addWidget(replaceTopRow);
    replaceMainLayout->addWidget(replaceBottomRow);
    replaceWidget->setLayout(replaceMainLayout);

    searchDock = new QDockWidget(tr("Find/Replace"), this);
    searchDock->setAllowedAreas(Qt::BottomDockWidgetArea);



    // A container widget to hold both find and replace widgets vertically
    QWidget *searchContainer = new QWidget(this);
    QVBoxLayout *dockLayout = new QVBoxLayout(searchContainer);

    // Add the findWidget and replaceWidget to this layout, but keep them hidden by default
    dockLayout->addWidget(findWidget);
    dockLayout->addWidget(replaceWidget);

    dockLayout->setContentsMargins(0, 0, 0, 0);
    dockLayout->setSpacing(0);

    // Initially, hide both toolbars so the dock doesn't take up space unnecessarily
    findWidget->hide();
    replaceWidget->hide();

    findWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    replaceWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    searchContainer->setLayout(dockLayout);
    searchDock->setWidget(searchContainer);

    // Add the dock to the bottom area of the main window
    addDockWidget(Qt::BottomDockWidgetArea, searchDock);

    // Optionally, you can keep the dock hidden until the user triggers "Find" or "Replace"
    searchDock->hide();

    // Connect search and replace signals to their respective slots if needed
    connect(findPrevButton, &QPushButton::clicked, this, &MainWindow::onFindPrevious);
    connect(findNextButton, &QPushButton::clicked, this, &MainWindow::onFindNext);
    connect(replacePrevButton, &QPushButton::clicked, this, &MainWindow::onReplacePrevious);
    connect(replaceNextButton, &QPushButton::clicked, this, &MainWindow::onReplaceNext);
    connect(replaceAllButton, &QPushButton::clicked, this, &MainWindow::onReplaceAll);

    connect(latexEditor->document(), &QTextDocument::contentsChange,
            this, &MainWindow::onContentsChange);
    createMenus();

    loadPreferences();

    QShortcut *ctrlYShortcut = new QShortcut(QKeySequence("Ctrl+Y"), this);
    ctrlYShortcut->setEnabled(false);

    // Initialize keychain actions
    keychainActions["Ctrl+f"]="\\frac{$1}{$2}";
    keychainActions["Ctrl+r"]="\\sqrt[$2]{$1}";
    keychainActions["Ctrl+h"]="^{$1}";
    keychainActions["Ctrl+l"]="_{$1}";
    keychainActions["Ctrl+i"]="\\int";
    keychainActions["Ctrl+7"]="\\sum";
    keychainActions["Ctrl+9"]="\\left($1\\right)";
    keychainActions["Ctrl+["]="\\left[$1\\right]";
    keychainActions["Ctrl+<"]="\\left<$1\\right>";
    keychainActions["Ctrl+{"]="\\left\\{$1\\right\\}";
    keychainActions["Ctrl+d"]="\\begin{align*}\n$1\n\\end{align}";
    keychainActions["Ctrl+\\\\"]="\\left|$1\\right|";
    keychainActions["Ctrl+||"]="\\lVert$1\\rVert";
    keychainActions["Ctrl+space"]="\\:";
    keychainActions["Ctrl+,"]="\\;";
    keychainActions["Ctrl+shift+space"]="\\!";
    keychainActions["Ctrl+b"]="\\nolimits";

    keychainActions["Ctrl+aa"]="\\aa";
    keychainActions["Ctrl+AA"]="\\AA";
    keychainActions["Ctrl+acute"]="\\acute";
    keychainActions["Ctrl+ae"]="\\ae";
    keychainActions["Ctrl+AE"]="\\AE";
    keychainActions["Ctrl+aleph"]="\\aleph";
    keychainActions["Ctrl+alpha"]="\\alpha";
    keychainActions["Ctrl+amalg"]="\\amalg";
    keychainActions["Ctrl+and"]="\\and";
    keychainActions["Ctrl+maketitle"]="\\maketitle";
    keychainActions["Ctrl+angle"]="\\angle";
    keychainActions["Ctrl+appendix"]="\\appendix";
    keychainActions["Ctrl+approx"]="\\approx";
    keychainActions["Ctrl+arccos"]="\\arccos";
    keychainActions["Ctrl+arcsin"]="\\arcsin";
    keychainActions["Ctrl+arctan"]="\\arctan";
    keychainActions["Ctrl+arg"]="\\arg";
    keychainActions["Ctrl+arraycolsep"]="\\arraycolsep";
    keychainActions["Ctrl+arrayrulewidth"]="\\arrayrulewidth";
    keychainActions["Ctrl+arraystretch"]="\\arraystretch";
    keychainActions["Ctrl+ast"]="\\ast";
    keychainActions["Ctrl+asymp"]="\\asymp";
    keychainActions["Ctrl+maketitle"]="\\maketitle";
    keychainActions["Ctrl+b"]="\\b";
    keychainActions["Ctrl+backslash"]="\\backslash";
    keychainActions["Ctrl+bar"]="\\bar{$1}";
    keychainActions["Ctrl+baselineskip"]="\\baselineskip";
    keychainActions["Ctrl+baselinestretch"]="\\baselinestretch";
    keychainActions["Ctrl+baselineskip"]="\\baselineskip";
    keychainActions["Ctrl+nonumber"]="\\nonumber";
    keychainActions["Ctrl+item"]="\\item";
    keychainActions["Ctrl+beta"]="\\beta";
    keychainActions["Ctrl+bf"]="\\bf";
    keychainActions["Ctrl+bigcap"]="\\bigcap";
    keychainActions["Ctrl+bigcirc"]="\\bigcirc";
    keychainActions["Ctrl+bigcup"]="\\bigcup";
    keychainActions["Ctrl+bigodot"]="\\bigodot";
    keychainActions["Ctrl+bigotimes"]="\\bigotimes";
    keychainActions["Ctrl+bigtriangledown"]="\\bigtriangledown";
    keychainActions["Ctrl+bigtriangleup"]="\\bigtriangleup";
    keychainActions["Ctrl+bigskip"]="\\bigskip";
    keychainActions["Ctrl+bigskipamount"]="\\bigskipamount";
    keychainActions["Ctrl+bigsqcup"]="\\bigsqcup";
    keychainActions["Ctrl+cap"]="\\cap";
    keychainActions["Ctrl+cdot"]="\\cdot";
    keychainActions["Ctrl+cdots"]="\\cdots";
    keychainActions["Ctrl+centering"]="\\centering";
    keychainActions["Ctrl+check"]="\\check{$1}";
    keychainActions["Ctrl+chi"]="\\chi";
    keychainActions["Ctrl+circ"]="\\circ";
    keychainActions["Ctrl+cleardoublepage"]="\\cleardoublepage";
    keychainActions["Ctrl+bigvee"]="\\bigvee";
    keychainActions["Ctrl+bmod"]="\\bmod";
    keychainActions["Ctrl+boldmath"]="\\boldmath";
    keychainActions["Ctrl+bot"]="\\bot";
    keychainActions["Ctrl+bottomfraction"]="\\bottomfraction";
    keychainActions["Ctrl+bowtie"]="\\bowtie";
    keychainActions["Ctrl+Box"]="\\Box";
    keychainActions["Ctrl+breve"]="\\breve";
    keychainActions["Ctrl+bullet"]="\\bullet";
    keychainActions["Ctrl+c"]="\\c";
    keychainActions["Ctrl+cal"]="\\cal";
    keychainActions["Ctrl+clearpage"]="\\clearpage";
    keychainActions["Ctrl+clubsuit"]="\\clubsuit";
    keychainActions["Ctrl+columnsep"]="\\columnsep";
    keychainActions["Ctrl+columnseprule"]="\\columnseprule";
    keychainActions["Ctrl+columnwidth"]="\\columnwidth";
    keychainActions["Ctrl+textwidth"]="\\textwidth";
    keychainActions["Ctrl+cong"]="\\cong";
    keychainActions["Ctrl+coprod"]="\\coprod";
    keychainActions["Ctrl+copyright"]="\\copyright";
    keychainActions["Ctrl+cos"]="\\cos";
    keychainActions["Ctrl+cosh"]="\\cosh";
    keychainActions["Ctrl+cot"]="\\cot";
    keychainActions["Ctrl+coth"]="\\coth";
    keychainActions["Ctrl+csc"]="\\csc";
    keychainActions["Ctrl+cup"]="\\cup";
    keychainActions["Ctrl+dot"]="\\dot{$1}";
    keychainActions["Ctrl+doteq"]="\\doteq";
    keychainActions["Ctrl+dag"]="\\dag";
    keychainActions["Ctrl+dagger"]="\\dagger";
    keychainActions["Ctrl+dashv"]="\\dashv";
    keychainActions["Ctrl+maketitle"]="\\maketitle";
    keychainActions["Ctrl+day"]="\\day";
    keychainActions["Ctrl+dblfloatpagefraction"]="\\dblfloatpagefraction";
    keychainActions["Ctrl+dblfloatsep"]="\\dblfloatsep";
    keychainActions["Ctrl+dbltextfloatsep"]="\\dbltextfloatsep";
    keychainActions["Ctrl+dbltopfraction"]="\\dbltopfraction";
    keychainActions["Ctrl+dotfill"]="\\dotfill";
    keychainActions["Ctrl+doublerulesep"]="\\doublerulesep";
    keychainActions["Ctrl+downarrow"]="\\downarrow";
    keychainActions["Ctrl+Downarrow"]="\\Downarrow";
    keychainActions["Ctrl+ell"]="\\ell";
    keychainActions["Ctrl+em"]="\\em";
    keychainActions["Ctrl+emptyset"]="\\emptyset";
    keychainActions["Ctrl+epsilon"]="\\epsilon";
    keychainActions["Ctrl+equiv"]="\\equiv";
    keychainActions["Ctrl+eta"]="\\eta";
    keychainActions["Ctrl+evensidemargin"]="\\evensidemargin";
    keychainActions["Ctrl+exists"]="\\exists";
    keychainActions["Ctrl+exp"]="\\exp";
    keychainActions["Ctrl+ddag"]="\\ddag";
    keychainActions["Ctrl+ddagger"]="\\ddagger";
    keychainActions["Ctrl+ddot"]="\\ddot{$1}";
    keychainActions["Ctrl+ddots"]="\\ddots";
    keychainActions["Ctrl+deg"]="\\deg";
    keychainActions["Ctrl+delta"]="\\delta";
    keychainActions["Ctrl+Delta"]="\\Delta";
    keychainActions["Ctrl+det"]="\\det";
    keychainActions["Ctrl+diamond"]="\\diamond";
    keychainActions["Ctrl+Diamond"]="\\Diamond";
    keychainActions["Ctrl+diamondsuit"]="\\diamondsuit";
    keychainActions["Ctrl+dim"]="\\dim";
    keychainActions["Ctrl+displaystyle"]="\\displaystyle";
    keychainActions["Ctrl+div"]="\\div";
    keychainActions["Ctrl+fboxrule"]="\\fboxrule";
    keychainActions["Ctrl+fboxsep"]="\\fboxsep";
    keychainActions["Ctrl+fbox"]="\\fbox";
    keychainActions["Ctrl+fill"]="\\fill";
    keychainActions["Ctrl+flat"]="\\flat";
    keychainActions["Ctrl+floatpagefraction"]="\\floatpagefraction";
    keychainActions["Ctrl+floatsep"]="\\floatsep";
    keychainActions["Ctrl+flushbottom"]="\\flushbottom";
    keychainActions["Ctrl+footheight"]="\\footheight";
    keychainActions["Ctrl+footnotemark"]="\\footnotemark";
    keychainActions["Ctrl+footnotesep"]="\\footnotesep";
    keychainActions["Ctrl+footnotesize"]="\\footnotesize";
    keychainActions["Ctrl+footskip"]="\\footskip";
    keychainActions["Ctrl+forall"]="\\forall";
    keychainActions["Ctrl+frown"]="\\frown";
    keychainActions["Ctrl+fussy"]="\\fussy";
    keychainActions["Ctrl+gamma"]="\\gamma";
    keychainActions["Ctrl+Gamma"]="\\Gamma";
    keychainActions["Ctrl+gcd"]="\\gcd";
    keychainActions["Ctrl+ge"]="\\ge";
    keychainActions["Ctrl+geq"]="\\geq";
    keychainActions["Ctrl+gets"]="\\gets";
    keychainActions["Ctrl+gg"]="\\gg";
    keychainActions["Ctrl+glossaryentry"]="\\glossaryentry";
    keychainActions["Ctrl+grave"]="\\grave";
    keychainActions["Ctrl+hat"]="\\hat{$1}";
    keychainActions["Ctrl+hbar"]="\\hbar";
    keychainActions["Ctrl+headheight"]="\\headheight";
    keychainActions["Ctrl+headsep"]="\\headsep";
    keychainActions["Ctrl+heartsuit"]="\\heartsuit";
    keychainActions["Ctrl+hfill"]="\\hfill";
    keychainActions["Ctrl+hline"]="\\hline";
    keychainActions["Ctrl+hom"]="\\hom";
    keychainActions["Ctrl+hookleftarrow"]="\\hookleftarrow";
    keychainActions["Ctrl+hookrightarrow"]="\\hookrightarrow";
    keychainActions["Ctrl+hrulefill"]="\\hrulefill";
    keychainActions["Ctrl+huge"]="\\huge";
    keychainActions["Ctrl+Huge"]="\\Huge";
    keychainActions["Ctrl+iff"]="\\iff";
    keychainActions["Ctrl+Im"]="\\Im";
    keychainActions["Ctrl+imath"]="\\imath";
    keychainActions["Ctrl+in"]="\\in";
    keychainActions["Ctrl+include"]="\\include";
    keychainActions["Ctrl+indexentry"]="\\indexentry";
    keychainActions["Ctrl+indexspace"]="\\indexspace";
    keychainActions["Ctrl+inf"]="\\inf";
    keychainActions["Ctrl+infty"]="\\infty";
    keychainActions["Ctrl+int"]="\\int";
    keychainActions["Ctrl+intextsep"]="\\intextsep";
    keychainActions["Ctrl+iota"]="\\iota";
    keychainActions["Ctrl+it"]="\\it";
    keychainActions["Ctrl+itemindent"]="\\itemindent";
    keychainActions["Ctrl+itemsep"]="\\itemsep";
    keychainActions["Ctrl+jmath"]="\\jmath";
    keychainActions["Ctrl+Join"]="\\Join";
    keychainActions["Ctrl+kappa"]="\\kappa";
    keychainActions["Ctrl+ker"]="\\ker";
    keychainActions["Ctrl+kill"]="\\kill";
    keychainActions["Ctrl+tabbing"]="\\tabbing";
    keychainActions["Ctrl+labelwidth"]="\\labelwidth";
    keychainActions["Ctrl+labelsep"]="\\labelsep";
    keychainActions["Ctrl+lambda"]="\\lambda";
    keychainActions["Ctrl+Lambda"]="\\Lambda";
    keychainActions["Ctrl+land"]="\\land";
    keychainActions["Ctrl+langle"]="\\langle";
    keychainActions["Ctrl+LARGE"]="\\LARGE";
    keychainActions["Ctrl+normalsize"]="\\normalsize";
    keychainActions["Ctrl+LaTeX"]="\\LaTeX";
    keychainActions["Ctrl+lbrace"]="\\lbrace";
    keychainActions["Ctrl+lbrack"]="\\lbrack";
    keychainActions["Ctrl+lceil"]="\\lceil";
    keychainActions["Ctrl+ldots"]="\\ldots";
    keychainActions["Ctrl+le"]="\\le";
    keychainActions["Ctrl+leadsto"]="\\leadsto";
    keychainActions["Ctrl+leftarrow"]="\\leftarrow";
    keychainActions["Ctrl+Leftarrow"]="\\Leftarrow";
    keychainActions["Ctrl+leftharpoondown"]="\\leftharpoondown";
    keychainActions["Ctrl+leftharpoonup"]="\\leftharpoonup";
    keychainActions["Ctrl+leftmargini"]="\\leftmargini";
    keychainActions["Ctrl+leftrightarrow"]="\\leftrightarrow";
    keychainActions["Ctrl+Leftrightarrow"]="\\Leftrightarrow";
    keychainActions["Ctrl+leq"]="\\leq";
    keychainActions["Ctrl+lfloor"]="\\lfloor";
    keychainActions["Ctrl+lg"]="\\lg";
    keychainActions["Ctrl+lhd"]="\\lhd";
    keychainActions["Ctrl+lim"]="\\lim";
    keychainActions["Ctrl+liminf"]="\\liminf";
    keychainActions["Ctrl+limsup"]="\\limsup";
    keychainActions["Ctrl+put"]="\\put";
    keychainActions["Ctrl+put"]="\\put";
    keychainActions["Ctrl+linewidth"]="\\linewidth";
    keychainActions["Ctrl+listoffigures"]="\\listoffigures";
    keychainActions["Ctrl+listoftables"]="\\listoftables";
    keychainActions["Ctrl+listparindent"]="\\listparindent";
    keychainActions["Ctrl+ll"]="\\ll";
    keychainActions["Ctrl+ln"]="\\ln";
    keychainActions["Ctrl+lnot"]="\\lnot";
    keychainActions["Ctrl+log"]="\\log";
    keychainActions["Ctrl+longleftarrow"]="\\longleftarrow";
    keychainActions["Ctrl+Longleftarrow"]="\\Longleftarrow";
    keychainActions["Ctrl+longleftrightarrow"]="\\longleftrightarrow";
    keychainActions["Ctrl+Longleftrightarrow"]="\\Longleftrightarrow";
    keychainActions["Ctrl+longmapsto"]="\\longmapsto";
    keychainActions["Ctrl+longrightarrow"]="\\longrightarrow";
    keychainActions["Ctrl+Longrightarrow"]="\\Longrightarrow";
    keychainActions["Ctrl+lor"]="\\lor";
    keychainActions["Ctrl+lq"]="\\lq";
    keychainActions["Ctrl+makeglossary"]="\\makeglossary";
    keychainActions["Ctrl+glossaryentry"]="\\glossaryentry";
    keychainActions["Ctrl+makeindex"]="\\makeindex";
    keychainActions["Ctrl+maketitle"]="\\maketitle";
    keychainActions["Ctrl+mapsto"]="\\mapsto";
    keychainActions["Ctrl+marginparpush"]="\\marginparpush";
    keychainActions["Ctrl+marginparsep"]="\\marginparsep";
    keychainActions["Ctrl+marginparwidth"]="\\marginparwidth";
    keychainActions["Ctrl+max"]="\\max";
    keychainActions["Ctrl+medskip"]="\\medskip";
    keychainActions["Ctrl+medskipamount"]="\\medskipamount";
    keychainActions["Ctrl+mho"]="\\mho";
    keychainActions["Ctrl+mid"]="\\mid";
    keychainActions["Ctrl+min"]="\\min";
    keychainActions["Ctrl+mit"]="\\mit";
    keychainActions["Ctrl+models"]="\\models";
    keychainActions["Ctrl+month"]="\\month";
    keychainActions["Ctrl+mp"]="\\mp";
    keychainActions["Ctrl+mu"]="\\mu";
    keychainActions["Ctrl+nabla"]="\\nabla";
    keychainActions["Ctrl+natural"]="\\natural";
    keychainActions["Ctrl+ne"]="\\ne";
    keychainActions["Ctrl+nearrow"]="\\nearrow";
    keychainActions["Ctrl+neg"]="\\neg";
    keychainActions["Ctrl+neq"]="\\neq";
    keychainActions["Ctrl+cs"]="\\cs";
    keychainActions["Ctrl+cs"]="\\cs";
    keychainActions["Ctrl+nl"]="\\nl";
    keychainActions["Ctrl+newline"]="\\newline";
    keychainActions["Ctrl+newpage"]="\\newpage";
    keychainActions["Ctrl+ni"]="\\ni";
    keychainActions["Ctrl+nofiles"]="\\nofiles";
    keychainActions["Ctrl+noindent"]="\\noindent";
    keychainActions["Ctrl+linebreak"]="\\linebreak";
    keychainActions["Ctrl+nonumber"]="\\nonumber";
    keychainActions["Ctrl+linebreak"]="\\linebreak";
    keychainActions["Ctrl+normalmarginpar"]="\\normalmarginpar";
    keychainActions["Ctrl+normalsize"]="\\normalsize";
    keychainActions["Ctrl+not"]="\\not";
    keychainActions["Ctrl+notin"]="\\notin";
    keychainActions["Ctrl+nu"]="\\nu";
    keychainActions["Ctrl+nwarrow"]="\\nwarrow";
    keychainActions["Ctrl+obeycr"]="\\obeycr";
    keychainActions["Ctrl+oddsidemargin"]="\\oddsidemargin";
    keychainActions["Ctrl+odot"]="\\odot";
    keychainActions["Ctrl+oe"]="\\oe";
    keychainActions["Ctrl+OE"]="\\OE";
    keychainActions["Ctrl+oint"]="\\oint";
    keychainActions["Ctrl+omega"]="\\omega";
    keychainActions["Ctrl+Omega"]="\\Omega";
    keychainActions["Ctrl+ominus"]="\\ominus";
    keychainActions["Ctrl+onecolumn"]="\\onecolumn";
    keychainActions["Ctrl+oplus"]="\\oplus";
    keychainActions["Ctrl+oslash"]="\\oslash";
    keychainActions["Ctrl+otimes"]="\\otimes";
    keychainActions["Ctrl+put"]="\\put";
    keychainActions["Ctrl+owns"]="\\owns";
    keychainActions["Ctrl+P"]="\\P";
    keychainActions["Ctrl+linebreak"]="\\linebreak";
    keychainActions["Ctrl+parallel"]="\\parallel";
    keychainActions["Ctrl+parindent"]="\\parindent";
    keychainActions["Ctrl+parsep"]="\\parsep";
    keychainActions["Ctrl+parskip"]="\\parskip";
    keychainActions["Ctrl+partial"]="\\partial";
    keychainActions["Ctrl+partopsep"]="\\partopsep";
    keychainActions["Ctrl+perp"]="\\perp";
    keychainActions["Ctrl+phi"]="\\phi";
    keychainActions["Ctrl+Phi"]="\\Phi";
    keychainActions["Ctrl+pi"]="\\pi";
    keychainActions["Ctrl+Pi"]="\\Pi";
    keychainActions["Ctrl+pm"]="\\pm";
    keychainActions["Ctrl+poptabs"]="\\poptabs";
    keychainActions["Ctrl+pounds"]="\\pounds";
    keychainActions["Ctrl+Pr"]="\\Pr";
    keychainActions["Ctrl+prec"]="\\prec";
    keychainActions["Ctrl+preceq"]="\\preceq";
    keychainActions["Ctrl+prime"]="\\prime";
    keychainActions["Ctrl+prod"]="\\prod";
    keychainActions["Ctrl+propto"]="\\propto";
    keychainActions["Ctrl+protect"]="\\protect";
    keychainActions["Ctrl+caption"]="\\caption";
    keychainActions["Ctrl+ps"]="\\ps";
    keychainActions["Ctrl+psi"]="\\psi";
    keychainActions["Ctrl+Psi"]="\\Psi";
    keychainActions["Ctrl+pushtabs"]="\\pushtabs";
    keychainActions["Ctrl+raggedbottom"]="\\raggedbottom";
    keychainActions["Ctrl+raggedleft"]="\\raggedleft";
    keychainActions["Ctrl+raggedright"]="\\raggedright";
    keychainActions["Ctrl+rangle"]="\\rangle";
    keychainActions["Ctrl+rbrace"]="\\rbrace";
    keychainActions["Ctrl+rbrack"]="\\rbrack";
    keychainActions["Ctrl+rceil"]="\\rceil";
    keychainActions["Ctrl+Re"]="\\Re";
    keychainActions["Ctrl+cs"]="\\cs";
    keychainActions["Ctrl+restorecr"]="\\restorecr";
    keychainActions["Ctrl+obeycr"]="\\obeycr";
    keychainActions["Ctrl+reversemarginpar"]="\\reversemarginpar";
    keychainActions["Ctrl+rfloor"]="\\rfloor";
    keychainActions["Ctrl+rhd"]="\\rhd";
    keychainActions["Ctrl+rho"]="\\rho";
    keychainActions["Ctrl+rightarrow"]="\\rightarrow";
    keychainActions["Ctrl+Rightarrow"]="\\Rightarrow";
    keychainActions["Ctrl+rightharpoondown"]="\\rightharpoondown";
    keychainActions["Ctrl+rightharpoonup"]="\\rightharpoonup";
    keychainActions["Ctrl+rightleftharpoons"]="\\rightleftharpoons";
    keychainActions["Ctrl+rightmargin"]="\\rightmargin";
    keychainActions["Ctrl+rm"]="\\rm";
    keychainActions["Ctrl+rq"]="\\rq";
    keychainActions["Ctrl+makebox"]="\\makebox";
    keychainActions["Ctrl+binname"]="\\binname";
    keychainActions["Ctrl+sc"]="\\sc";
    keychainActions["Ctrl+scriptsize"]="\\scriptsize";
    keychainActions["Ctrl+scriptstyle"]="\\scriptstyle";
    keychainActions["Ctrl+scriptscriptstyle"]="\\scriptscriptstyle";
    keychainActions["Ctrl+searrow"]="\\searrow";
    keychainActions["Ctrl+sec"]="\\sec";
    keychainActions["Ctrl+nl"]="\\nl";
    keychainActions["Ctrl+setminus"]="\\setminus";
    keychainActions["Ctrl+nl"]="\\nl";
    keychainActions["Ctrl+sf"]="\\sf";
    keychainActions["Ctrl+sharp"]="\\sharp";
    keychainActions["Ctrl+sigma"]="\\sigma";
    keychainActions["Ctrl+Sigma"]="\\Sigma";
    keychainActions["Ctrl+sim"]="\\sim";
    keychainActions["Ctrl+simeq"]="\\simeq";
    keychainActions["Ctrl+sin"]="\\sin";
    keychainActions["Ctrl+sinh"]="\\sinh";
    keychainActions["Ctrl+sl"]="\\sl";
    keychainActions["Ctrl+sloppy"]="\\sloppy";
    keychainActions["Ctrl+small"]="\\small";
    keychainActions["Ctrl+smallint"]="\\smallint";
    keychainActions["Ctrl+smallskip"]="\\smallskip";
    keychainActions["Ctrl+smallskipamount"]="\\smallskipamount";
    keychainActions["Ctrl+smile"]="\\smile";
    keychainActions["Ctrl+spadesuit"]="\\spadesuit";
    keychainActions["Ctrl+sqcap"]="\\sqcap";
    keychainActions["Ctrl+sqcup"]="\\sqcup";
    keychainActions["Ctrl+sqsubset"]="\\sqsubset";
    keychainActions["Ctrl+sqsubseteq"]="\\sqsubseteq";
    keychainActions["Ctrl+sqsupset"]="\\sqsupset";
    keychainActions["Ctrl+sqsupseteq"]="\\sqsupseteq";
    keychainActions["Ctrl+ss"]="\\ss";
    keychainActions["Ctrl+star"]="\\star";
    keychainActions["Ctrl+stop"]="\\stop";
    keychainActions["Ctrl+subset"]="\\subset";
    keychainActions["Ctrl+subseteq"]="\\subseteq";
    keychainActions["Ctrl+succ"]="\\succ";
    keychainActions["Ctrl+succeq"]="\\succeq";
    keychainActions["Ctrl+sum"]="\\sum";
    keychainActions["Ctrl+sup"]="\\sup";
    keychainActions["Ctrl+supset"]="\\supset";
    keychainActions["Ctrl+supseteq"]="\\supseteq";
    keychainActions["Ctrl+surd"]="\\surd";
    keychainActions["Ctrl+swarrow"]="\\swarrow";
    keychainActions["Ctrl+tabbingsep"]="\\tabbingsep";
    keychainActions["Ctrl+tabcolsep"]="\\tabcolsep";
    keychainActions["Ctrl+tableofcontents"]="\\tableofcontents";
    keychainActions["Ctrl+tan"]="\\tan";
    keychainActions["Ctrl+tanh"]="\\tanh";
    keychainActions["Ctrl+tau"]="\\tau";
    keychainActions["Ctrl+TeX"]="\\TeX";
    keychainActions["Ctrl+textfloatsep"]="\\textfloatsep";
    keychainActions["Ctrl+textfraction"]="\\textfraction";
    keychainActions["Ctrl+textheight"]="\\textheight";
    keychainActions["Ctrl+textstyle"]="\\textstyle";
    keychainActions["Ctrl+textwidth"]="\\textwidth";
    keychainActions["Ctrl+maketitle"]="\\maketitle";
    keychainActions["Ctrl+theta"]="\\theta";
    keychainActions["Ctrl+Theta"]="\\Theta";
    keychainActions["Ctrl+thicklines"]="\\thicklines";
    keychainActions["Ctrl+thinlines"]="\\thinlines";
    keychainActions["Ctrl+thinspace"]="\\thinspace";
    keychainActions["Ctrl+pagestyle"]="\\pagestyle";
    keychainActions["Ctrl+tilde"]="\\tilde{$1}";
    keychainActions["Ctrl+times"]="\\times";
    keychainActions["Ctrl+tiny"]="\\tiny";
    keychainActions["Ctrl+maketitle"]="\\maketitle";
    keychainActions["Ctrl+to"]="\\to";
    keychainActions["Ctrl+today"]="\\today";
    keychainActions["Ctrl+top"]="\\top";
    keychainActions["Ctrl+topfraction"]="\\topfraction";
    keychainActions["Ctrl+topmargin"]="\\topmargin";
    keychainActions["Ctrl+topsep"]="\\topsep";
    keychainActions["Ctrl+topskip"]="\\topskip";
    keychainActions["Ctrl+triangle"]="\\triangle";
    keychainActions["Ctrl+triangleleft"]="\\triangleleft";
    keychainActions["Ctrl+triangleright"]="\\triangleright";
    keychainActions["Ctrl+tt"]="\\tt";
    keychainActions["Ctrl+cs"]="\\cs";
    keychainActions["Ctrl+unboldmath"]="\\unboldmath";
    keychainActions["Ctrl+unitlength"]="\\unitlength";
    keychainActions["Ctrl+unlhd"]="\\unlhd";
    keychainActions["Ctrl+unrhd"]="\\unrhd";
    keychainActions["Ctrl+uparrow"]="\\uparrow";
    keychainActions["Ctrl+Uparrow"]="\\Uparrow";
    keychainActions["Ctrl+updownarrow"]="\\updownarrow";
    keychainActions["Ctrl+Updownarrow"]="\\Updownarrow";
    keychainActions["Ctrl+uplus"]="\\uplus";
    keychainActions["Ctrl+upsilon"]="\\upsilon";
    keychainActions["Ctrl+Upsilon"]="\\Upsilon";
    keychainActions["Ctrl+varepsilon"]="\\varepsilon";
    keychainActions["Ctrl+varphi"]="\\varphi";
    keychainActions["Ctrl+varpi"]="\\varpi";
    keychainActions["Ctrl+varrho"]="\\varrho";
    keychainActions["Ctrl+varsigma"]="\\varsigma";
    keychainActions["Ctrl+vartheta"]="\\vartheta";
    keychainActions["Ctrl+vdash"]="\\vdash";
    keychainActions["Ctrl+vdots"]="\\vdots";
    keychainActions["Ctrl+vec"]="\\vec{$1}";
    keychainActions["Ctrl+put"]="\\put";
    keychainActions["Ctrl+put"]="\\put";
    keychainActions["Ctrl+vee"]="\\vee";
    keychainActions["Ctrl+vert"]="\\vert";
    keychainActions["Ctrl+Vert"]="\\Vert";
    keychainActions["Ctrl+vfill"]="\\vfill";
    keychainActions["Ctrl+wedge"]="\\wedge";
    keychainActions["Ctrl+wp"]="\\wp";
    keychainActions["Ctrl+wr"]="\\wr";
    keychainActions["Ctrl+xi"]="\\xi";
    keychainActions["Ctrl+Xi"]="\\Xi";
    keychainActions["Ctrl+year"]="\\year";
    keychainActions["Ctrl+zeta"]="\\zeta";
    keychainActions["Ctrl+rm"]="\\rm";
    keychainActions["Ctrl+it"]="\\it";
    keychainActions["Ctrl+bf"]="\\bf";
    keychainActions["Ctrl+sl"]="\\sl";
    keychainActions["Ctrl+dag"]="\\dag";
    keychainActions["Ctrl+sf"]="\\sf";
    keychainActions["Ctrl+sc"]="\\sc";
    keychainActions["Ctrl+tt"]="\\tt";
    keychainActions["Ctrl+ddag"]="\\ddag";
    keychainActions["Ctrl+P"]="\\P";
    keychainActions["Ctrl+oe"]="\\oe";
    keychainActions["Ctrl+aa"]="\\aa";
    keychainActions["Ctrl+Xi"]="\\Xi";
    keychainActions["Ctrl+OE"]="\\OE";
    keychainActions["Ctrl+AA"]="\\AA";
    keychainActions["Ctrl+L"]="\\L";
    keychainActions["Ctrl+ae"]="\\ae";
    keychainActions["Ctrl+ss"]="\\ss";
    keychainActions["Ctrl+AE"]="\\AE";
    keychainActions["Ctrl+pm"]="\\pm";
    keychainActions["Ctrl+mp"]="\\mp";
    keychainActions["Ctrl+sum"]="\\sum";
    keychainActions["Ctrl+prod"]="\\prod";
    keychainActions["Ctrl+setminus"]="\\setminus";
    keychainActions["Ctrl+cdot"]="\\cdot";
    keychainActions["Ctrl+times"]="\\times";
    keychainActions["Ctrl+ast"]="\\ast";
    keychainActions["Ctrl+int"]="\\int";
    keychainActions["Ctrl+star"]="\\star";
    keychainActions["Ctrl+diamond"]="\\diamond";
    keychainActions["Ctrl+circ"]="\\circ";
    keychainActions["Ctrl+oint"]="\\oint";
    keychainActions["Ctrl+bigodot"]="\\bigodot";
    keychainActions["Ctrl+bullet"]="\\bullet";
    keychainActions["Ctrl+div"]="\\div";
    keychainActions["Ctrl+lhd"]="\\lhd";
    keychainActions["Ctrl+vee"]="\\vee";
    keychainActions["Ctrl+wedge"]="\\wedge";
    keychainActions["Ctrl+oplus"]="\\oplus";
    keychainActions["Ctrl+ominus"]="\\ominus";
    keychainActions["Ctrl+otimes"]="\\otimes";
    keychainActions["Ctrl+oslash"]="\\oslash";
    keychainActions["Ctrl+leq"]="\\leq";
    keychainActions["Ctrl+prec"]="\\prec";
    keychainActions["Ctrl+preceq"]="\\preceq";
    keychainActions["Ctrl+ll"]="\\ll";
    keychainActions["Ctrl+subset"]="\\subset";
    keychainActions["Ctrl+subseteq"]="\\subseteq";
    keychainActions["Ctrl+sqsubset"]="\\sqsubset";
    keychainActions["Ctrl+sqsubseteq"]="\\sqsubseteq";
    keychainActions["Ctrl+in"]="\\in";
    keychainActions["Ctrl+vdash"]="\\vdash";
    keychainActions["Ctrl+smile"]="\\smile";
    keychainActions["Ctrl+frown"]="\\frown";
    keychainActions["Ctrl+neq"]="\\neq";
    keychainActions["Ctrl+equiv"]="\\equiv";
    keychainActions["Ctrl+sim"]="\\sim";
    keychainActions["Ctrl+simeq"]="\\simeq";
    keychainActions["Ctrl+asymp"]="\\asymp";
    keychainActions["Ctrl+bigoplus"]="\\bigoplus";
    keychainActions["Ctrl+lfloor"]="\\lfloor";
    keychainActions["Ctrl+rfloor"]="\\rfloor";
    keychainActions["Ctrl+lceil"]="\\lceil";
    keychainActions["Ctrl+rceil"]="\\rceil";
    keychainActions["Ctrl+langle"]="\\langle";
    keychainActions["Ctrl+vert"]="\\vert";
    keychainActions["Ctrl+uparrow"]="\\uparrow";
    keychainActions["Ctrl+downarrow"]="\\downarrow";
    keychainActions["Ctrl+updownarrow"]="\\updownarrow";
    keychainActions["Ctrl+arccos"]="\\arccos";
    keychainActions["Ctrl+csc"]="\\csc";
    keychainActions["Ctrl+ker"]="\\ker";
    keychainActions["Ctrl+leftarrow"]="\\leftarrow";
    keychainActions["Ctrl+Leftarrow"]="\\Leftarrow";
    keychainActions["Ctrl+rightarrow"]="\\rightarrow";
    keychainActions["Ctrl+Rightarrow"]="\\Rightarrow";
    keychainActions["Ctrl+leftrightarrow"]="\\leftrightarrow";
    keychainActions["Ctrl+Leftrightarrow"]="\\Leftrightarrow";
    keychainActions["Ctrl+mapsto"]="\\mapsto";
    keychainActions["Ctrl+hookleftarrow"]="\\hookleftarrow";
    keychainActions["Ctrl+leftharpoonup"]="\\leftharpoonup";
    keychainActions["Ctrl+leftharpoondown"]="\\leftharpoondown";
    keychainActions["Ctrl+rightleftharpoons"]="\\rightleftharpoons";
    keychainActions["Ctrl+uparrow"]="\\uparrow";
    keychainActions["Ctrl+Uparrow"]="\\Uparrow";
    keychainActions["Ctrl+downarrow"]="\\downarrow";
    keychainActions["Ctrl+Downarrow"]="\\Downarrow";
    keychainActions["Ctrl+updownarrow"]="\\updownarrow";
    keychainActions["Ctrl+aleph"]="\\aleph";
    keychainActions["Ctrl+hbar"]="\\hbar";
    keychainActions["Ctrl+ell"]="\\ell";
    keychainActions["Ctrl+wp"]="\\wp";
    keychainActions["Ctrl+Re"]="\\Re";
    keychainActions["Ctrl+Im"]="\\Im";
    keychainActions["Ctrl+partial"]="\\partial";
    keychainActions["Ctrl+infty"]="\\infty";
    keychainActions["Ctrl+Box"]="\\Box";
    keychainActions["Ctrl+forall"]="\\forall";
    keychainActions["Ctrl+exists"]="\\exists";
    keychainActions["Ctrl+neg"]="\\neg";
    keychainActions["Ctrl+flat"]="\\flat";
    keychainActions["Ctrl+natural"]="\\natural";
    keychainActions["Ctrl+frac"]="\\frac{$1}{$2}";
    keychainActions["Ctrl+sqrt"]="\\sqrt[$2]{$1}";
    keychainActions["Ctrl+xrightarrow"]="\\xrightarrow{$1}";
    keychainActions["Ctrl+xleftarrow"]="\\xleftarrow{$1}";
    keychainActions["Ctrl+mathring"]="\\mathring{$1}";
    keychainActions["Ctrl+widehat"]="\\widehat{$1}";
    keychainActions["Ctrl+pmod"]="\\pmod{$1}";
    keychainActions["Ctrl+overset"]="\\overset{$2}&{$1}";
    keychainActions["Ctrl+underset"]="\\underset{$2}&{$1}";
    keychainActions["Ctrl+abs"]="\\abs{$1}";
    keychainActions["Ctrl+norm"]="\\norm{$1}";
    keychainActions["Ctrl+mathbb"]="\\mathbb{$1}";
    keychainActions["Ctrl+boldsymbol"]="\\boldsymbol{$1}";
    keychainActions["Ctrl+text"]="\\text{$1}";

    // Populate Greek letter map
    greekLetterMap["a"]="\\alpha";
    greekLetterMap["b"]="\\beta";
    greekLetterMap["g"]="\\gamma";
    greekLetterMap["shift+g"]="\\Gamma";
    greekLetterMap["d"]="\\delta";
    greekLetterMap["shift+d"]="\\Delta";
    greekLetterMap["e"]="\\epsilon";
    greekLetterMap["shift+e"]="\\varepsilon";
    greekLetterMap["z"]="\\zeta";
    greekLetterMap["h"]="\\eta";
    greekLetterMap["y"]="\\theta";
    greekLetterMap["shift+z"]="\\vartheta";
    greekLetterMap["shift+y"]="\\Theta";
    greekLetterMap["i"]="\\iota";
    greekLetterMap["k"]="\\kappa";
    greekLetterMap["shift+k"]="\\Kappa";
    greekLetterMap["l"]="\\lambda";
    greekLetterMap["L"]="\\Lambda";
    greekLetterMap["m"]="\\mu";
    greekLetterMap["n"]="\\nu";
    greekLetterMap["x"]="\\xi";
    greekLetterMap["shift+x"]="\\Xi";
    greekLetterMap["p"]="\\pi";
    greekLetterMap["shift+p"]="\\Pi";
    greekLetterMap["v"]="\\varpi";
    greekLetterMap["r"]="\\rho";
    greekLetterMap["shift+r"]="\\Rho";
    greekLetterMap["s"]="\\sigma";
    greekLetterMap["shift+s"]="\\Sigma";
    greekLetterMap["shift+t"]="\\varsigma";
    greekLetterMap["t"]="\\tau";
    greekLetterMap["u"]="\\upsilon";
    greekLetterMap["shift+u"]="\\Upsilon";
    greekLetterMap["f"]="\\phi";
    greekLetterMap["shift+f"]="\\Phi";
    greekLetterMap["j"]="\\varphi";
    greekLetterMap["q"]="\\chi";
    greekLetterMap["c"]="\\psi";
    greekLetterMap["shift+c"]="\\Psi";
    greekLetterMap["w"]="\\omega";
    greekLetterMap["shift+w"]="\\Omega";
    greekLetterMap["shift+i"]="\\digamma";
    greekLetterMap["shift+h"]="\\nabla";
    greekLetterMap["left"] = "\\leftarrow";
    greekLetterMap["shift+left"] = "\\longleftarrow";
    greekLetterMap["right"] = "\\rightarrow";
    greekLetterMap["shift+right"] = "\\longrightarrow";
    greekLetterMap["up"] = "\\uparrow";
    greekLetterMap["shift+up"] = "\\longuparrow";
    greekLetterMap["down"] = "\\downarrow";
    greekLetterMap["shift+down"] = "\\longdownarrow";
    greekLetterMap["pageup"] = "\\nwarrow";
    greekLetterMap["pagedown"] = "\\swarrow";
    greekLetterMap["home"] = "\\nearrow";
    greekLetterMap["end"] = "\\searrow";

}


MainWindow::~MainWindow() {
    // No need to delete pointers; Qt handles this automatically
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    static const QSet<QString> ctrlShortcuts = {"a", "x", "c", "v", "z"};
    static const QSet<QString> arrowKeys = {"left", "right", "up", "down", "pageup", "pagedown", "home", "end"};

    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_Y) {
            qDebug() << "[DEBUG] ShortcutOverride for Ctrl+Y ignored, allowing propagation.";

            keyPressEvent(keyEvent);
            return true; // Let the event continue to keyPressEvent
        }
    }

    if (obj == latexEditor && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Save state before any text modification
        if (!(keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() != Qt::Key_Shift &&
            keyEvent->key() != Qt::Key_Alt && keyEvent->key() != Qt::Key_Meta &&
            !arrowKeys.contains(QKeySequence(keyEvent->key()).toString().toLower())) {

            // Save the current state before processing this key press
            saveState();
        }

        // Get symbolic representation of the key
        QString symbolicKey = QKeySequence(keyEvent->key()).toString().toLower();

        // Debugging: Log the key press
        qDebug() << "[DEBUG] Event filter symbolicKey:" << symbolicKey;

        // Check if we're in a math environment
        std::string text = latexEditor->toPlainText().toStdString();
        QTextCursor cursor = latexEditor->textCursor();
        int currentPosition = cursor.position(); // Get the integer position

        // Handle Arrow Keys
        if (keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_Left) {
            if (navigation::in_math(text, currentPosition)) {
                int newPosition = currentPosition; // Initialize with current position

                // Determine new position based on Ctrl/Shift
                if (keyEvent->key() == Qt::Key_Right) {
                    if (keyEvent->modifiers() & Qt::ControlModifier) {
                        newPosition = navigation::next_term_special(text, currentPosition);
                    } else {
                        newPosition = navigation::next_term(text, currentPosition);
                    }
                } else if (keyEvent->key() == Qt::Key_Left) {
                    if (keyEvent->modifiers() & Qt::ControlModifier) {
                        newPosition = navigation::previous_term_special(text, currentPosition);
                    } else {
                        newPosition = navigation::previous_term(text, currentPosition);
                    }
                }

                // Apply highlighting if Shift is pressed
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    cursor.setPosition(newPosition, QTextCursor::KeepAnchor); // Highlight from anchor to new position
                } else {
                    cursor.setPosition(newPosition, QTextCursor::MoveAnchor); // Move without highlighting
                }

                // Update the cursor in the editor
                latexEditor->setTextCursor(cursor);
                return true; // Mark as handled
            }
        }

        // Handle Ctrl+Shortcuts
        if (keyEvent->modifiers() & Qt::ControlModifier) {
            if (ctrlShortcuts.contains(symbolicKey)) {
                // Append or start a new keychain dynamically
                if (keyChain.isEmpty()) {
                    keyChain = "Ctrl+" + symbolicKey;
                } else {
                    keyChain += symbolicKey;
                }

                qDebug() << "[DEBUG] Ctrl+Key handled. Key Chain:" << keyChain;

                return true; // Block default behavior
            }
        }

        if (symbolicKey == "backspace") {
            // Preserve the existing behavior for Greek mode and key chains
            if (isGreekMode || !keyChain.isEmpty()) {
                if (!keyChain.isEmpty()) {
                    keyChain.chop(1);  // Remove the last character
                }
                qDebug() << "[DEBUG] Backspace pressed. Updated Key Chain:" << keyChain;

                // Mark the event as handled
                return true;
            }

            // New behavior for Backspace in math block
            if (navigation::in_math(text, currentPosition)) {
                int previousPosition = navigation::previous_term(text, currentPosition);
                if (previousPosition != currentPosition) {
                    cursor.setPosition(previousPosition, QTextCursor::KeepAnchor); // Highlight to previous term
                    cursor.removeSelectedText(); // Remove the highlighted text
                    latexEditor->setTextCursor(cursor); // Update the cursor
                    return true; // Mark the event as handled
                }
            }

            return false; // Default behavior if no conditions are met
        }

        // Handle Delete
        if (keyEvent->key() == Qt::Key_Delete && !(keyEvent->modifiers() & Qt::ControlModifier)) {
            // Check if we are in a math block
            if (navigation::in_math(text, currentPosition)) {
                int nextPosition = navigation::next_term(text, currentPosition);
                if (nextPosition != currentPosition) {
                    cursor.setPosition(nextPosition, QTextCursor::KeepAnchor); // Highlight to next term
                    cursor.removeSelectedText(); // Remove the highlighted text
                    latexEditor->setTextCursor(cursor); // Update the cursor
                    return true; // Mark the event as handled
                }
            }
            return false; // Default behavior if conditions aren't met
        }

        // Handle Greek mode within the filter
        if (isGreekMode) {
            // Check for Shift or Caps Lock modifiers
            bool isShiftActive = (keyEvent->modifiers() & Qt::ShiftModifier) != 0;
            bool capsLockActive = isCapsLockActive();

            // Adjust key for Shift or Caps Lock
            if (isShiftActive || capsLockActive) {
                symbolicKey = "shift+" + symbolicKey;
            }

            // Pass the symbolicKey to Greek mode handling
            handleGreekMode(symbolicKey);
            return true; // Mark the event as handled
        }

        if (isMatrixMode) {
            // Check for Shift or Caps Lock modifiers
            bool isShiftActive = (keyEvent->modifiers() & Qt::ShiftModifier) != 0;
            bool capsLockActive = isCapsLockActive();

            // Adjust key for Shift or Caps Lock
            if (isShiftActive || capsLockActive) {
                symbolicKey = "shift+" + symbolicKey;
            }

            // Pass the symbolicKey to Greek mode handling
            handleMatrixMode(symbolicKey);
            return true; // Mark the event as handled
        }

        // Allow other events to propagate
        return false;
    }

    // Pass unhandled events to the base class
    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
    // Log raw key details
    qDebug() << "[DEBUG] Raw key:" << event->key();                 // Key code
    qDebug() << "[DEBUG] Symbolic key:" << QKeySequence(event->key()).toString(); // Symbolic representation
    qDebug() << "[DEBUG] Key text:" << event->text();              // Text representation

    // Track active keys to prevent duplicates
    if (activeKeys.contains(event->key())) {
        return;
    }
    activeKeys.insert(event->key());

    // Get symbolic representation of the key
    QString symbolicKey = QKeySequence(event->key()).toString().toLower();

    if (symbolicKey == "f5") {
        compile();
        qDebug() << "Compile triggered";
        return; // Shortcut handled
    }

    // Handle Greek mode
    if (isGreekMode) {
        handleGreekMode(symbolicKey);                 // Process Greek key mapping
        return;                                   // Skip further processing
    }

    // Handle Greek mode
    if (isMatrixMode) {
        handleMatrixMode(symbolicKey);                 // Process Greek key mapping
        return;                                   // Skip further processing
    }

    // Handle Backspace using symbolicKey
    if (symbolicKey == "backspace") {
        if (!keyChain.isEmpty()) {
            keyChain.chop(1);  // Remove the last character
        }
        qDebug() << "[DEBUG] Backspace pressed. Updated Key Chain:" << keyChain;
        statusBar()->showMessage("Key Chain: " + keyChain);
        return;  // Skip further processing for Backspace
    }

    // Handle modifiers separately
    if (event->key() == Qt::Key_Control) {
        activeModifiers.insert(Qt::ControlModifier);
        if (!keyChain.contains("Ctrl")) {
            keyChain += "Ctrl+";
        }
        return;
    }
    if (event->key() == Qt::Key_Alt) {
        activeModifiers.insert(Qt::AltModifier);
        if (!keyChain.contains("Alt")) {
            keyChain += "Alt+";
        }
        return;
    }
    if (event->key() == Qt::Key_Meta) {
        activeModifiers.insert(Qt::MetaModifier);
        if (!keyChain.contains("Meta")) {
            keyChain += "Meta+";
        }
        return;
    }
    if (event->key() == Qt::Key_Shift) {
        // Skip adding Shift as a symbolic key
        return;
    }

    // Detect modifiers
    bool isShiftActive = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool capsLockActive = isCapsLockActive();  // Platform-specific check

    // Handle symbolic keys
    if (!symbolicKey.isEmpty()) {
        // Determine case based on Shift and Caps Lock states
        bool isUpperCase = (isShiftActive != capsLockActive);  // XOR logic
        if (isUpperCase) {
            keyChain += symbolicKey.toUpper();
            qDebug() << "[DEBUG] Uppercase symbolic key added:" << symbolicKey.toUpper();
        } else {
            keyChain += symbolicKey.toLower();
            qDebug() << "[DEBUG] Lowercase symbolic key added:" << symbolicKey.toLower();
        }
    }

    // Update the status bar for the current key chain
    statusBar()->showMessage("Key Chain: " + keyChain);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    // Remove the released key from the active set
    activeKeys.remove(event->key());

    // Check if a modifier key is released
    if (event->key() == Qt::Key_Control) {
        activeModifiers.remove(Qt::ControlModifier);
    } else if (event->key() == Qt::Key_Alt) {
        activeModifiers.remove(Qt::AltModifier);
    } else if (event->key() == Qt::Key_Meta) {
        activeModifiers.remove(Qt::MetaModifier);
    }

    // Automatic pairing for specific keys when Ctrl is not active
    // Automatic pairing for specific keys when Ctrl is not active
    if (!(event->modifiers() & Qt::ControlModifier)) {
        QString symbolicKey = QKeySequence(event->key()).toString().toLower();
        QString insertion;

        if (symbolicKey == "(") {
            insertion = ")";
        } else if (symbolicKey == "[") {
            insertion = "]";
        } else if (symbolicKey == "{") {
            insertion = "}";
        }

        if (!insertion.isEmpty()) {
            QTextCursor cursor = latexEditor->textCursor();

            // Check if the character already exists before inserting
            QString previousChar = cursor.block().text().mid(cursor.position() - 1, 1);
            if (previousChar == symbolicKey) {
                cursor.insertText(insertion);  // Insert the closing pair only
                cursor.movePosition(QTextCursor::Left); // Move cursor between the pair
                latexEditor->setTextCursor(cursor); // Update the cursor
                qDebug() << "[DEBUG] Automatic pairing inserted closing:" << insertion;
                return; // Stop further processing
            }
        }
    }

    // Append special symbolic keys to the key chain
    switch (event->key()) {
    case Qt::Key_Backslash: keyChain += "\\"; break;
    case Qt::Key_Less: keyChain += "<"; break;
    case Qt::Key_Greater: keyChain += ">"; break;
    case Qt::Key_BraceLeft: keyChain += "{"; break;
    case Qt::Key_BraceRight: keyChain += "}"; break;
    case Qt::Key_Bar: keyChain += "|"; break;
    default: break;
    }

    // Enter Greek mode on Ctrl+g
    if (keyChain == "Ctrl+g") {
        keyChain.clear();
        isGreekMode = true;
        isMatrixMode = false;
        statusBar()->showMessage("Greek mode activated");
        qDebug() << "[DEBUG] Greek mode activated";
        return;
    }

    // Enter Matrix mode on Ctrl+m
    if (keyChain == "Ctrl+m") {
        keyChain.clear();
        isGreekMode = false;
        isMatrixMode = true;
        statusBar()->showMessage("Matrix mode activated");
        qDebug() << "[DEBUG] Matrix mode activated";
        return;
    }

    // Finalize the key chain only when all modifiers are released
    if (activeModifiers.isEmpty() && !keyChain.isEmpty()) {
        processKeychain();
    }
}

void MainWindow::processKeychain() {
    // Handle specific editor actions
    if (keyChain == "Ctrl+c") {
        copy();  // Perform copy action
    } else if (keyChain == "Ctrl+x") {
        cut();  // Perform cut action
    } else if (keyChain == "Ctrl+v") {
        paste();  // Perform paste action
    } else if (keyChain == "Ctrl+z") {
        undo();  // Perform undo action
    } else if (keyChain == "Ctrl+y") {
        redo();  // Perform redo action
    } else if (keyChain == "Ctrl+b") {
        insertFontStyle();
    } else if (keyChain == "Ctrl+q") {
        findText();
    } else if (keyChain == "Ctrl+w") {
        replaceText();
    } else if (keyChain == "Ctrl+=") {
        zoomIn();
    } else if (keyChain == "Ctrl+-") {
        zoomOut();
    } else if (keyChain == "Ctrl+s") {
        saveFile();
    } else if (keyChain == "Ctrl+S") {
        saveFileAs();
    } else if (keyChain == "Ctrl+a") {
        latexEditor->selectAll();
    } else if (keyChain == "Ctrl+t") {
        toggleMathTextMode();
    } else if (keychainActions.contains(keyChain)) {
        // Handle custom LaTeX insertion actions
        QString latexCommand = keychainActions[keyChain];
        processAndInsertText(latexCommand);
        qDebug() << "[DEBUG] Action executed for keychain:" << keyChain
                 << "-> Inserted:" << latexCommand;
    } else {
        // No action defined for the keychain
        qDebug() << "[DEBUG] No action defined for keychain:" << keyChain;
    }

    // Clear the keychain after processing
    keyChain.clear();
    statusBar()->showMessage("Ready");
}

void MainWindow::handleGreekMode(QString key) {
    // Check if the key exists in the Greek letter map
    if (greekLetterMap.contains(key)) {
        QString latexCommand = greekLetterMap[key];
        processAndInsertText(latexCommand);
        qDebug() << "[DEBUG] Inserted Greek letter:" << latexCommand;
        statusBar()->showMessage("Inserted: " + latexCommand);

        // Exit Greek mode after a valid letter
        isGreekMode = false;
        statusBar()->showMessage("Ready");
        qDebug() << "[DEBUG] Greek mode deactivated";
        return;
    }

    // Handle Escape key to exit Greek mode
    if (key == "esc") {
        isGreekMode = false;
        statusBar()->showMessage("Ready");
        qDebug() << "[DEBUG] Greek mode deactivated (escaped)";
    } else {
        qDebug() << "[DEBUG] Invalid key in Greek mode:" << key;
    }
}

void MainWindow::handleMatrixMode(QString key) {
    QString latexContent = latexEditor->toPlainText();
    QTextCursor cursor = latexEditor->textCursor();
    int cursorPos = cursor.position();

    try {
        if (key == "m") {
            // Insert matrix
            latexContent = matrixmode::insertMatrix(latexContent, cursorPos);
            int insertedLength = QStringLiteral("\\begin{matrix}\n$1 & $2 \\\\\n$3 & $4\n\\end{matrix}").length();
            cursorPos += insertedLength;
            cursor.setPosition(cursorPos);
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Inserted matrix.");
            isMatrixMode = false;
        }
        else if (key == "t") {
            // Insert table
            latexContent = matrixmode::insertTable(latexContent, cursorPos);
            int insertedLength = QStringLiteral("\\begin{tabular}{cc}\n$1 & $2 \\\\\n$3 & $4\n\\end{tabular}").length();
            cursorPos += insertedLength;
            cursor.setPosition(cursorPos);
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Inserted table.");
            isMatrixMode = false;
        }
        else if (key == "up") {
            // Insert row before current row
            auto [newContent, newPos] = matrixmode::insert_row_before_cursor(latexContent, cursorPos);
            latexContent = newContent;
            cursorPos = newPos;
            cursor.setPosition(cursorPos);
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Inserted row before current row.");
            isMatrixMode = false;
        }
        else if (key == "down") {
            // Insert row after current row
            auto [newContent, newPos] = matrixmode::insert_row_after_cursor(latexContent, cursorPos);
            latexContent = newContent;
            cursorPos = newPos;
            cursor.setPosition(cursorPos);
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Inserted row after current row.");
            isMatrixMode = false;
        }
        else if (key == "left") {
            // Insert column before current column
            QString newContent = matrixmode::insert_column_before_cursor(latexContent, cursorPos);
            latexContent = newContent;
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Inserted column before current column.");
            isMatrixMode = false;
        }
        else if (key == "right") {
            // Insert column after current column
            QString newContent = matrixmode::insert_column_after_cursor(latexContent, cursorPos);
            latexContent = newContent;
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Inserted column after current column.");
            isMatrixMode = false;
        }
        else if (key == "r") {
            // Delete current row
            auto [newContent, newPos] = matrixmode::delete_row_at_cursor(latexContent, cursorPos);
            latexContent = newContent;
            cursorPos = newPos;
            cursor.setPosition(cursorPos);
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Deleted current row.");
            isMatrixMode = false;
        }
        else if (key == "c") {
            // Delete current column
            auto [newContent, newPos] = matrixmode::delete_column_at_cursor(latexContent, cursorPos);
            latexContent = newContent;
            cursorPos = newPos;
            cursor.setPosition(cursorPos);
            latexEditor->setPlainText(latexContent);
            latexEditor->setTextCursor(cursor);
            statusBar()->showMessage("Deleted current column.");
            isMatrixMode = false;
        }
        else {
            // Invalid key, just exit matrix mode
            qDebug() << "[DEBUG] Invalid key in Matrix mode:" << key;
            isMatrixMode = false;
        }
    } catch (const std::exception &e) {
        qDebug() << "[ERROR]" << e.what();
        statusBar()->showMessage("Matrix operation error: " + QString(e.what()));
        isMatrixMode = false;
    }
}

void MainWindow::createMenus() {
    // File menu
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New"), this, &MainWindow::newFile);
    fileMenu->addAction(tr("&Open"), this, &MainWindow::openFile);
    fileMenu->addAction(tr("&Close"), this, &MainWindow::closeFile);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Save"), this, &MainWindow::saveFile);
    fileMenu->addAction(tr("Save &As"), this, &MainWindow::saveFileAs);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Compile"), this, &MainWindow::compile);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Import"), this, &MainWindow::importFile);
    fileMenu->addAction(tr("&Export"), this, &MainWindow::exportFile);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Preview"), this, &MainWindow::preview);
    fileMenu->addAction(tr("&Print"), this, &MainWindow::printFile);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Preferences"), this, &MainWindow::preferences);
    fileMenu->addAction(tr("&Recent"), this, &MainWindow::recentFiles);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &MainWindow::exitApp);

    // Edit menu
    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Undo"), this, &MainWindow::undo);
    editMenu->addAction(tr("&Redo"), this, &MainWindow::redo);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Cut"), this, &MainWindow::cut);
    editMenu->addAction(tr("&Copy"), this, &MainWindow::copy);
    editMenu->addAction(tr("&Paste"), this, &MainWindow::paste);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Delete"), this, &MainWindow::deleteContent);
    editMenu->addAction(tr("&Select All"), this, &MainWindow::selectAll);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Find"), this, &MainWindow::findText);
    editMenu->addAction(tr("&Replace"), this, &MainWindow::replaceText);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Spelling"), this, &MainWindow::spellCheck);

    // Insert menu
    insertMenu = menuBar()->addMenu(tr("&Insert"));
    insertMenu->addAction(tr("Math/&Text"), this, &MainWindow::insertMathText);
    insertMenu->addSeparator();
    insertMenu->addAction(tr("&Fraction"), this, &MainWindow::insertFraction);
    insertMenu->addAction(tr("&Radical"), this, &MainWindow::insertRadical);
    insertMenu->addAction(tr("&Superscript"), this, &MainWindow::insertSuperscript);
    insertMenu->addAction(tr("&Subscript"), this, &MainWindow::insertSubscript);
    insertMenu->addAction(tr("&Display"), this, &MainWindow::insertDisplay);
    insertMenu->addSeparator();
    insertMenu->addAction(tr("&Operator"), this, &MainWindow::insertOperator);
    insertMenu->addAction(tr("&Bracket"), this, &MainWindow::insertBracket);
    insertMenu->addAction(tr("&Matrix"), this, &MainWindow::insertMatrix);
    insertMenu->addAction(tr("&Math Name"), this, &MainWindow::insertMathName);
    insertMenu->addAction(tr("&Binomial"), this, &MainWindow::insertBinomial);
    insertMenu->addAction(tr("&Label"), this, &MainWindow::insertLabel);
    insertMenu->addAction(tr("&Decoration"), this, &MainWindow::insertDecoration);
    insertMenu->addAction(tr("&Font Style"), this, &MainWindow::insertFontStyle);
    insertMenu->addSeparator();
    insertMenu->addAction(tr("&Spacing"), this, &MainWindow::insertSpacing);

    // View menu
    viewMenu = menuBar()->addMenu(tr("&View"));
    // Create an action group for mutually exclusive selection
    QActionGroup *fontSizeGroup = new QActionGroup(this);
    fontSizeGroup->setExclusive(true);
    // Common font sizes
    QStringList fontSizes = {"8", "10", "12 (Default)", "14", "16", "18", "20"};
    for (const QString &size : fontSizes) {
        QAction *action = viewMenu->addAction(size + " pt");
        action->setCheckable(true); // Make the action checkable
        fontSizeGroup->addAction(action);

        // Connect to set the font size
        connect(action, &QAction::triggered, [this, size]() {
            bool ok;
            int points = size.split(" ").first().toInt(&ok); // Extract point size
            if (ok) setFontSize(points);
        });

        // Default to 12 pt being selected
        if (size.contains("12")) action->setChecked(true);
    }
    viewMenu->addSeparator();
    // Custom font size
    QAction *customAction = viewMenu->addAction(tr("Custom..."));
    connect(customAction, &QAction::triggered, this, &MainWindow::customFontSize);
    viewMenu->addSeparator();
    // Add Zoom controls to the View menu
    viewMenu->addSeparator();
    QAction *zoomInAction = viewMenu->addAction(tr("Zoom In"));
    QAction *zoomOutAction = viewMenu->addAction(tr("Zoom Out"));

    // Connect these actions to their respective slots
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);

    // Add reset zoom if desired
    QAction *resetZoomAction = viewMenu->addAction(tr("Reset Zoom"));
    connect(resetZoomAction, &QAction::triggered, this, &MainWindow::resetZoom);
    viewMenu->addSeparator();
    // Refresh action
    viewMenu->addAction(tr("Refresh"), this, &MainWindow::refreshView);

    // Compute menu
    computeMenu = menuBar()->addMenu(tr("&Compute"));
    computeMenu->addAction(tr("&Evaluate"), this, &MainWindow::evaluate);
    computeMenu->addAction(tr("Evaluate &Numerically"), this, &MainWindow::evaluateNumerically);
    computeMenu->addAction(tr("&Simplify"), this, &MainWindow::simplify);
    computeMenu->addAction(tr("&Combine"), this, &MainWindow::combine);
    computeMenu->addAction(tr("&Factor"), this, &MainWindow::factor);
    computeMenu->addAction(tr("&Expand"), this, &MainWindow::expand);
    computeMenu->addAction(tr("&Rewrite"), this, &MainWindow::rewrite);
    computeMenu->addAction(tr("&Check equality"), this, &MainWindow::checkEquality);
    computeMenu->addAction(tr("&Solve"), this, &MainWindow::solve);
    computeMenu->addSeparator();
    computeMenu->addAction(tr("&Plot 2D"), this, &MainWindow::plot2D);
    computeMenu->addAction(tr("&Plot 3D"), this, &MainWindow::plot3D);
    computeMenu->addSeparator();
    computeMenu->addAction(tr("&Settings"), this, &MainWindow::computeSettings);
}

// File menu actions
void MainWindow::newFile() {
    if (checkUnsavedChanges()) {
        latexEditor->setPlainText(latexTemplate);
        currentFile.clear();
        isModified = false;
        statusBar()->showMessage("New file created.");
    }
}


void MainWindow::openFile() {
    if (!checkUnsavedChanges()) return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("LaTeX Files (*.tex);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file."));
        return;
    }

    QTextStream in(&file);
    latexEditor->setPlainText(in.readAll());
    file.close();

    currentFile = fileName;
    isModified = false;

    // Attempt to load the corresponding PDF
    QString pdfFile = QFileInfo(fileName).absolutePath() + "/" + QFileInfo(fileName).baseName() + ".pdf";
    loadPDF(pdfFile);
    updateRecentFiles(fileName);
}


void MainWindow::saveFile() {
    if (currentFile.isEmpty()) {
        saveFileAs();
    } else {
        writeToFile(currentFile);
    }
    updateRecentFiles(currentFile);
}

void MainWindow::saveFileAs() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File As"), QString(), tr("LaTeX Files (*.tex);;All Files (*)"));
    if (fileName.isEmpty()) return;

    writeToFile(fileName);
    updateRecentFiles(fileName);
}

void MainWindow::closeFile() {
    if (checkUnsavedChanges()) {
        latexEditor->clear();
        currentFile.clear();
        isModified = false;
        statusBar()->showMessage("File closed.");
    }
}
void MainWindow::compile() {
    if (currentFile.isEmpty()) {
        statusBar()->showMessage("No LaTeX file to compile.");
        return;
    }

    if (!pdflatexProcess) {
        pdflatexProcess = new QProcess(this);
        connect(pdflatexProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &MainWindow::reloadPDF);
    }

    QString filePath = currentFile;
    QString fileDir = QFileInfo(filePath).absolutePath();
    QString fileName = QFileInfo(filePath).fileName();

    // Check if latexInterpreterPath is set
    if (latexInterpreterPath.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("LaTeX interpreter path is not set in preferences."));
        return;
    }

    statusBar()->showMessage("Compiling...");
    QStringList arguments = {fileName};

    pdflatexProcess->setWorkingDirectory(fileDir);
    pdflatexProcess->start(latexInterpreterPath, arguments);

    if (!pdflatexProcess->waitForStarted()) {
        statusBar()->showMessage("Failed to start pdflatex.");
        qDebug() << "[ERROR] Failed to start pdflatex process.";
    }
}
void MainWindow::importFile() { qDebug() << "Import file triggered."; }
void MainWindow::exportFile() { qDebug() << "Export file triggered."; }
void MainWindow::preview() {
    if (currentFile.isEmpty()) {
        statusBar()->showMessage("No file to preview.");
        return;
    }

    // Generate the PDF file path
    QString pdfFilePath = QFileInfo(currentFile).absolutePath() + "/" +
                          QFileInfo(currentFile).baseName() + ".pdf";

    if (!QFile::exists(pdfFilePath)) {
        statusBar()->showMessage("PDF file does not exist. Please compile first.");
        qDebug() << "[ERROR] PDF file not found:" << pdfFilePath;
        return;
    }

    // Open the preview window
    PreviewWindow *previewWindow = new PreviewWindow(pdfFilePath, this);
    previewWindow->setAttribute(Qt::WA_DeleteOnClose); // Clean up when closed
    previewWindow->show();

    statusBar()->showMessage("Preview opened.");
}
void MainWindow::printFile() {
    if (currentFile.isEmpty()) {
        statusBar()->showMessage("No file to print.");
        return;
    }

    // Generate the PDF file path
    QString pdfFilePath = QFileInfo(currentFile).absolutePath() + "/" +
                          QFileInfo(currentFile).baseName() + ".pdf";

    if (!QFile::exists(pdfFilePath)) {
        statusBar()->showMessage("PDF file does not exist. Please compile first.");
        qDebug() << "[ERROR] PDF file not found:" << pdfFilePath;
        return;
    }

    // Load the PDF document
    QPdfDocument pdfDocument;
    if (pdfDocument.load(pdfFilePath) != QPdfDocument::Error::None) {
        statusBar()->showMessage("Failed to load PDF for printing.");
        qDebug() << "[ERROR] Failed to load PDF file for printing:" << pdfFilePath;
        return;
    }

    // Set up the printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::NativeFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage("Printing cancelled.");
        return;
    }

    QPainter painter;
    if (!painter.begin(&printer)) {
        statusBar()->showMessage("Failed to open printer.");
        qDebug() << "[ERROR] Could not start printing.";
        return;
    }

    // Print each page of the PDF
    int pageCount = pdfDocument.pageCount();
    QSize printerPageSize = QSize(printer.pageRect(QPrinter::DevicePixel).width(),
                                  printer.pageRect(QPrinter::DevicePixel).height());

    for (int i = 0; i < pageCount; ++i) {
        // Render the page as an image
        QPdfDocumentRenderOptions renderOptions;
        QImage pageImage = pdfDocument.render(i, printerPageSize, renderOptions);

        if (!pageImage.isNull()) {
            // Draw the rendered image onto the printer's page
            QRect targetRect(0, 0, printer.pageRect(QPrinter::DevicePixel).width(),
                             printer.pageRect(QPrinter::DevicePixel).height());
            painter.drawImage(targetRect, pageImage);

            // Move to the next printer page, if applicable
            if (i < pageCount - 1)
                printer.newPage();
        } else {
            qDebug() << "[ERROR] Failed to render page:" << i;
        }
    }

    painter.end();
    statusBar()->showMessage("Printing complete.");
    qDebug() << "[INFO] Printing completed successfully.";
}
void MainWindow::preferences() {
    QJsonObject preferences = PreferencesManager::loadPreferences();
    PreferencesDialog dialog(preferences, this);
    if (dialog.exec() == QDialog::Accepted) {
        qDebug() << "Dialog accepted.";
        // Update member variables with new preferences
        preferences = dialog.getUpdatedPreferences();
        // Apply the new preferences to the application
        qDebug() << "About to apply preferences";
        PreferencesManager::savePreferences(preferences);
        qDebug() << "Preferences saved.";
    }
}
void MainWindow::recentFiles() {
    QStringList recentFilesList = loadRecentFiles();

    RecentFilesDialog dialog(this);
    dialog.setRecentFiles(recentFilesList);

    connect(&dialog, &RecentFilesDialog::fileSelected, this, &MainWindow::openRecentFile);
    connect(&dialog, &RecentFilesDialog::clearRequested, this, &MainWindow::clearRecentFiles);

    dialog.exec();
}
void MainWindow::exitApp() {
    if (checkUnsavedChanges()) {  // Prompt the user to save changes if necessary
        QApplication::quit();    // Close the application
    }
}

void MainWindow::undo() { applyState(undoStack, redoStack, "Undo"); }
void MainWindow::redo() { applyState(redoStack, undoStack, "Redo"); }

void MainWindow::cut() {
    latexEditor->cut();
}

void MainWindow::copy() {
    latexEditor->copy();
}

void MainWindow::paste() {
    latexEditor->paste();
}
void MainWindow::deleteContent() {
    // Check if there is any selected text
    QTextCursor cursor = latexEditor->textCursor();
    if (cursor.hasSelection()) {
        cursor.removeSelectedText();  // Remove the selected text
        latexEditor->setTextCursor(cursor);  // Update the editor with the modified cursor
    } else {
        statusBar()->showMessage("No text selected to delete.");
    }
}
void MainWindow::selectAll() {
    latexEditor->selectAll();  // Select all the text in the editor
}
void MainWindow::findText() {
    // Show the dock if it's hidden
    // This ensures the dock and its widgets are visible
    searchDock->show();

    // Show findWidget and hide replaceWidget
    replaceWidget->hide();
    findWidget->show();
    findLineEdit->setFocus();
}
void MainWindow::replaceText() {
    // Show the dock if it's hidden
    searchDock->show();

    // Show replaceWidget and hide findWidget
    findWidget->hide();
    replaceWidget->show();
    replaceFindLineEdit->setFocus();
}
void MainWindow::spellCheck() { qDebug() << "Spelling check triggered."; }

// Insert menu actions
void MainWindow::insertMathText() { qDebug() << "Insert math/text triggered."; }
void MainWindow::insertFraction() {
    processAndInsertText("\\frac{$1}{$2}");
}

void MainWindow::insertRadical() {
    processAndInsertText("\\sqrt{$1}");
}

void MainWindow::insertSuperscript() {
    processAndInsertText("^{$1}");
}

void MainWindow::insertSubscript() {
    processAndInsertText("_{$1}");
}

void MainWindow::insertDisplay() {
    processAndInsertText("\\[ $1 \\]");
}

void MainWindow::insertMatrix() {
    processAndInsertText("\\begin{bmatrix}\n$1 & $2 \\\\\n$3 & $4\n\\end{bmatrix}");
}

void MainWindow::insertTable() {
    processAndInsertText("\\begin{array}{cc}\n$1 & $2 \\\\\n$3 & $4\n\\end{array}");
}

void MainWindow::insertBinomial() {
    processAndInsertText("\\binom{$1}{$2}");
}

void MainWindow::insertOperator() {
    processAndInsertText("_{$1}^{$2}");
}

void MainWindow::insertBracket() {
    qDebug() << "Insert bracket triggered.";
    BracketChooserDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QString leftBracket = dialog.getLeftBracket();
        QString rightBracket = dialog.getRightBracket();

        QString latexCode = dialog.toLaTeX(leftBracket, rightBracket);

        processAndInsertText(latexCode);

        qDebug() << "Generated LaTeX:" << latexCode;
        // Use latexCode in your LaTeX editor
    }
}
void MainWindow::insertMathName() {
    MathNameDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString mathName = dialog.getSelectedMathName();
        QString limitPlacement = dialog.getLimitPlacement();

        // Insert the selected math name with limit placement
        QString latexCode;
        if (limitPlacement == "above-below") {
            latexCode = "\\" + mathName + "\\limits";
        } else if (limitPlacement == "at-right") {
            latexCode = "\\" + mathName + "\\nolimits";
        } else {
            latexCode = "\\" + mathName;
        }

        processAndInsertText(latexCode);

        qDebug() << "[DEBUG] Inserted math name:" << latexCode;
    }
}
void MainWindow::insertLabel() {
    LabelDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString labelType = dialog.getSelectedLabelType();

        // Generate the corresponding LaTeX code
        QString latexCode;
        if (labelType == "overset") {
            latexCode = "\\overset{$1}&{$2}";
        } else if (labelType == "underset") {
            latexCode = "\\underset{$1}&{$2}";
        }

        processAndInsertText(latexCode);

        qDebug() << "[DEBUG] Inserted label:" << latexCode;
    }
}
void MainWindow::insertDecoration() {
    DecoratorDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString latexCode = dialog.getSelectedDecorator();
        if (!latexCode.isEmpty()) {
            processAndInsertText(latexCode);

            qDebug() << "[DEBUG] Inserted decoration:" << latexCode;
        }
    }
}
void MainWindow::insertSpacing() {
    SpacingDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString latexCode = dialog.getSelectedSpacing();
        if (!latexCode.isEmpty()) {
            // Insert the LaTeX code into the editor
            QTextCursor cursor = latexEditor->textCursor();
            cursor.insertText(latexCode);
            latexEditor->setTextCursor(cursor);

            qDebug() << "[DEBUG] Inserted spacing:" << latexCode;
        }
    }
}
void MainWindow::insertFontStyle() {
    FontDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString selectedFontCommand = dialog.getSelectedFontCommand();
        if (!selectedFontCommand.isEmpty()) {
            QString latexCode = selectedFontCommand + "{$1}";

            processAndInsertText(latexCode);

            qDebug() << "[DEBUG] Inserted font style:" << latexCode;
        } else {
            qDebug() << "[DEBUG] No font style selected.";
        }
    }
}
void MainWindow::insertNote() { qDebug() << "Insert note triggered."; }
void MainWindow::insertFormula() { qDebug() << "Insert formula triggered."; }
void MainWindow::insertHyperlink() { qDebug() << "Insert hyperlink triggered."; }
void MainWindow::insertMarker() { qDebug() << "Insert marker triggered."; }
void MainWindow::insertHtmlField() { qDebug() << "Insert HTML field triggered."; }

// View menu actions
void MainWindow::setFontSize(int points) {
    QFont font = latexEditor->font();
    font.setPointSize(points);
    latexEditor->setFont(font);

    qDebug() << "Font size set to" << points << "pt.";

    // Update the checkmark for the selected size
    foreach (QAction *action, viewMenu->actions()) {
        if (action->isCheckable()) {
            // Match the action's text with the selected size
            if (action->text().startsWith(QString::number(points) + " pt")) {
                action->setChecked(true);
            }
        }
    }
}
void MainWindow::customFontSize() {
    bool ok;
    int points = QInputDialog::getInt(this, tr("Custom Font Size"),
                                      tr("Enter font size (points):"),
                                      12,  // Default value
                                      6,   // Min value
                                      72,  // Max value
                                      1,   // Step
                                      &ok);
    if (ok) {
        setFontSize(points);
    }
}
void MainWindow::refreshView() {
    // Check if there’s a LaTeX document to reload
    if (currentFile.isEmpty()) {
        statusBar()->showMessage("No document to refresh.");
        qDebug() << "[DEBUG] No document to refresh.";
        return;
    }

    // Reload the LaTeX source file
    QFile file(currentFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        latexEditor->setPlainText(in.readAll());
        file.close();
        qDebug() << "[DEBUG] LaTeX document reloaded from:" << currentFile;
    } else {
        statusBar()->showMessage("Failed to reload LaTeX file.");
        qDebug() << "[ERROR] Failed to reload LaTeX file:" << currentFile;
    }

    // Reload the corresponding PDF file
    QString pdfFilePath = QFileInfo(currentFile).absolutePath() + "/" + QFileInfo(currentFile).baseName() + ".pdf";
    if (QFile::exists(pdfFilePath)) {
        if (pdfDocument->load(pdfFilePath) == QPdfDocument::Error::None) {
            pdfView->setDocument(pdfDocument);
            pdfView->setPageMode(QPdfView::PageMode::MultiPage);
            statusBar()->showMessage("PDF document reloaded.");
            qDebug() << "[DEBUG] PDF document reloaded from:" << pdfFilePath;
        } else {
            statusBar()->showMessage("Failed to reload PDF document.");
            qDebug() << "[ERROR] Failed to reload PDF document:" << pdfFilePath;
        }
    } else {
        statusBar()->showMessage("PDF file does not exist: " + pdfFilePath);
        qDebug() << "[ERROR] PDF file does not exist:" << pdfFilePath;
    }
}

// Compute menu actions
void MainWindow::evaluate() {
    const std::string operation = "evaluate";
    processLatexOperation(operation);
}

void MainWindow::evaluateNumerically() {
    const std::string operation = "evaluate numerically";
    processLatexOperation(operation);
}

void MainWindow::combine() {
    const std::string operation = "combine";
    processLatexOperation(operation);
}

void MainWindow::simplify() {
    const std::string operation = "simplify";
    processLatexOperation(operation);
}

void MainWindow::factor() {
    const std::string operation = "factor";
    processLatexOperation(operation);
}

void MainWindow::expand() {
    const std::string operation = "expand";
    processLatexOperation(operation);
}

void MainWindow::rewrite() {
    const std::string operation = "rewrite";
    processLatexOperation(operation);
}

void MainWindow::checkEquality() {
    const std::string operation = "check equality";
    processLatexOperation(operation);
}

void MainWindow::solve() {
    const std::string operation = "solve";
    processLatexOperation(operation);
}
void MainWindow::plot2D() { qDebug() << "Plot 2D triggered."; }
void MainWindow::plot3D() { qDebug() << "Plot 3D triggered."; }
void MainWindow::computeSettings() { qDebug() << "Compute settings triggered."; }

void MainWindow::loadPreferences() {
    QJsonObject preferences = PreferencesManager::loadPreferences();

    // Load font size
    int fontSize = preferences["defaultFontSize"].toInt(12);
    setFontSize(fontSize);

    // Load LaTeX interpreter and MATLAB paths
    latexInterpreterPath = preferences["latexInterpreterPath"].toString();
    matlabPath = preferences["matlabPath"].toString();

    // Load recent files
    QJsonArray recentFiles = preferences["recentFiles"].toArray();
    for (const QJsonValue &file : recentFiles) {
        recentFilesList.append(file.toString());
    }
}

void MainWindow::writeToFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not save file."));
        return;
    }

    QTextStream out(&file);

    // Replace internal <PLACEHOLDER_i_j> markers with $j for saving
    QString content = latexEditor->toPlainText();
    QRegularExpression placeholderRegex(R"(<PLACEHOLDER_\d+_(\d+)>)");
    content.replace(placeholderRegex, R"($\1)");

    out << content;
    file.close();

    statusBar()->showMessage(tr("Saved file: %1").arg(fileName));
}

// Check for unsaved changes before performing an action
bool MainWindow::checkUnsavedChanges() {
    if (!isModified) return true;

    auto result = QMessageBox::warning(this, tr("Unsaved Changes"),
                                       tr("The document has unsaved changes. Do you want to save them?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (result == QMessageBox::Yes) {
        saveFile();
        return !isModified;  // Ensure saving was successful
    }
    return result != QMessageBox::Cancel;
}

void MainWindow::loadDefaultTemplate() {
    latexTemplate = R"(%
\documentclass[12pt]{article}
\usepackage[margin=1in]{geometry}
\usepackage{graphicx}
\usepackage{amsfonts}
\usepackage{amssymb}
\usepackage{amsmath}

\begin{document}

% Your content here

\end{document}
)";
}

void MainWindow::saveState() {
    QString currentState = latexEditor->toPlainText();

    // Only save if the state is actually different
    if (!undoStack.isEmpty() && undoStack.last() == currentState) {
        qDebug() << "[DEBUG] saveState: Current state is identical to last state. Skipping save.";
        return;
    }

    // Save to undo stack
    undoStack.push_back(currentState);
    qDebug() << "[DEBUG] saveState: State saved to undo stack. Undo stack size:" << undoStack.size();
    qDebug() << "[DEBUG] saveState: State saved:\n" << currentState;

    // Clear redo stack because redo history becomes invalid after a new change
    redoStack.clear();
    qDebug() << "[DEBUG] saveState: Redo stack cleared.";
}

void MainWindow::applyState(QVector<QString>& sourceStack, QVector<QString>& targetStack, const QString& action) {
    if (!sourceStack.isEmpty()) {
        // Save the current cursor position
        QTextCursor cursor = latexEditor->textCursor();
        int currentCursorPosition = cursor.position();

        // Save the current state to the target stack
        QString currentState = latexEditor->toPlainText();
        targetStack.push_back(currentState);

        // Retrieve the new state from the source stack
        QString newState = sourceStack.takeLast();
        qDebug() << "[DEBUG]" << action << ": State popped from source stack:\n" << newState;

        // Compare current text with new state
        QString currentText = latexEditor->toPlainText();
        qDebug() << "[DEBUG] Current text before update:\n" << currentText;
        qDebug() << "[DEBUG] Text to update to:\n" << newState;

        // Update the editor with the new state
        latexEditor->blockSignals(true);
        latexEditor->setPlainText(newState);
        latexEditor->blockSignals(false);

        // Restore the cursor position
        int newCursorPos = (currentCursorPosition < newState.length())
                               ? currentCursorPosition
                               : static_cast<int>(newState.length());
        cursor.setPosition(newCursorPos);
        latexEditor->setTextCursor(cursor);

        // Force a UI refresh
        latexEditor->repaint();
        qDebug() << "[DEBUG] Cursor position after update:" << latexEditor->textCursor().position();
    } else {
        qDebug() << "[DEBUG]" << action << ": Source stack is empty. No action performed.";
    }
}

void MainWindow::loadPDF(const QString &filePath) {
    // Check if the file exists before attempting to load it
    if (QFile::exists(filePath)) {
        qDebug() << "Got here0\n";
        // Try to load the PDF document
        if (pdfDocument->load(filePath) == QPdfDocument::Error::None) {
            qDebug() << "Got here!\n";
            pdfView->setDocument(pdfDocument);
            pdfView->setPageMode(QPdfView::PageMode::MultiPage); // Example: SinglePage mode
            statusBar()->showMessage("PDF loaded: " + filePath);
        } else {
            statusBar()->showMessage("Failed to load PDF: " + filePath);
            pdfView->setDocument(nullptr); // Clear the view on failure
        }
    } else {
        // Handle the case where the file does not exist
        statusBar()->showMessage("PDF file does not exist: " + filePath);
        pdfView->setDocument(nullptr); // Clear the view
    }
}
void MainWindow::reloadPDF() {
    QString pdfFilePath = QFileInfo(currentFile).absolutePath() + "/" +
                          QFileInfo(currentFile).baseName() + ".pdf";

    if (QFile::exists(pdfFilePath)) {
        // Reload PDF into QPdfView
        if (pdfDocument->load(pdfFilePath) == QPdfDocument::Error::None) {
            pdfView->setDocument(pdfDocument);
            pdfView->setPageMode(QPdfView::PageMode::MultiPage);
            statusBar()->showMessage("Compilation complete. PDF reloaded.");
            qDebug() << "[INFO] PDF reloaded successfully:" << pdfFilePath;
        } else {
            statusBar()->showMessage("Failed to reload PDF.");
            qDebug() << "[ERROR] Failed to load PDF file:" << pdfFilePath;
        }
    } else {
        statusBar()->showMessage("Compilation failed: PDF file not generated.");
        qDebug() << "[ERROR] PDF file not found after compilation.";
    }
}

void MainWindow::onFindNext() {
    QString searchText = findLineEdit->text();
    if (searchText.isEmpty()) return;

    // Get the document from the editor
    QTextDocument *document = latexEditor->document();
    QTextCursor cursor = latexEditor->textCursor();

    // Move the cursor to the next occurrence of searchText
    QTextDocument::FindFlags flags;
    // No flags means forward search
    cursor = document->find(searchText, cursor, flags);

    if (!cursor.isNull()) {
        // Found occurrence
        latexEditor->setTextCursor(cursor);
    } else {
        statusBar()->showMessage(tr("No more occurrences of '%1'").arg(searchText));
    }
}

void MainWindow::onFindPrevious() {
    QString searchText = findLineEdit->text();
    if (searchText.isEmpty()) return;

    QTextDocument *document = latexEditor->document();
    QTextCursor cursor = latexEditor->textCursor();

    // Set the FindBackward flag to search backwards
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    cursor = document->find(searchText, cursor, flags);

    if (!cursor.isNull()) {
        latexEditor->setTextCursor(cursor);
    } else {
        statusBar()->showMessage(tr("No previous occurrences of '%1'").arg(searchText));
    }
}

void MainWindow::onReplaceNext() {
    QString searchText = replaceFindLineEdit->text();
    QString replaceText = replaceLineEdit->text();
    if (searchText.isEmpty()) return;

    // First, attempt to find next occurrence:
    QTextDocument *document = latexEditor->document();
    QTextCursor cursor = latexEditor->textCursor();
    cursor = document->find(searchText, cursor);

    if (!cursor.isNull()) {
        // Found an occurrence; replace it
        cursor.beginEditBlock();
        cursor.removeSelectedText();
        cursor.insertText(replaceText);
        cursor.endEditBlock();

        latexEditor->setTextCursor(cursor);
    } else {
        statusBar()->showMessage(tr("No more occurrences of '%1' to replace").arg(searchText));
    }
}

void MainWindow::onReplacePrevious() {
    QString searchText = replaceFindLineEdit->text();
    QString replaceText = replaceLineEdit->text();
    if (searchText.isEmpty()) return;

    QTextDocument *document = latexEditor->document();
    QTextCursor cursor = latexEditor->textCursor();
    cursor = document->find(searchText, cursor, QTextDocument::FindBackward);

    if (!cursor.isNull()) {
        // Found an occurrence; replace it
        cursor.beginEditBlock();
        cursor.removeSelectedText();
        cursor.insertText(replaceText);
        cursor.endEditBlock();

        latexEditor->setTextCursor(cursor);
    } else {
        statusBar()->showMessage(tr("No previous occurrences of '%1' to replace").arg(searchText));
    }
}

void MainWindow::onReplaceAll() {
    QString searchText = replaceFindLineEdit->text();
    QString replaceText = replaceLineEdit->text();
    if (searchText.isEmpty()) return;

    QTextDocument *document = latexEditor->document();
    QTextCursor cursor(document);
    int replacements = 0;

    cursor.beginEditBlock();
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = document->find(searchText, cursor);
        if (!cursor.isNull()) {
            cursor.removeSelectedText();
            cursor.insertText(replaceText);
            replacements++;
        }
    }
    cursor.endEditBlock();

    if (replacements <= 0) {
        statusBar()->showMessage(tr("No occurrences of '%1' found").arg(searchText));
    }
}
void MainWindow::zoomIn() {
    double currentZoom = pdfView->zoomFactor(); // Get the current zoom factor
    pdfView->setZoomFactor(currentZoom + 0.1);  // Increase zoom by 10%
    statusBar()->showMessage(tr("Zoomed In: %1%").arg((currentZoom + 0.1) * 100));
}

void MainWindow::zoomOut() {
    double currentZoom = pdfView->zoomFactor(); // Get the current zoom factor
    pdfView->setZoomFactor(currentZoom - 0.1);  // Decrease zoom by 10%
    statusBar()->showMessage(tr("Zoomed Out: %1%").arg((currentZoom - 0.1) * 100));
}

void MainWindow::resetZoom() {
    pdfView->setZoomFactor(1.0);  // Reset zoom to default (100%)
    statusBar()->showMessage(tr("Zoom Reset to 100%"));
}
void MainWindow::processLatexOperation(const std::string& operation) {
    QTextCursor cursor = latexEditor->textCursor();

    if (!cursor.hasSelection()) {
        statusBar()->showMessage("Error: No text selected.");
        return;
    }

    QString selectedText = cursor.selectedText();
    if (selectedText.isEmpty()) {
        statusBar()->showMessage("Error: No text selected.");
        return;
    }

    try {
        // Generate prompt
        std::string prompt = matlabutils::generatePrompt(operation, selectedText.toStdString());

        // Retrieve API key and MATLAB path from preferences
        std::string apiKey = PreferencesManager::getApiKey().toStdString();
        if (apiKey.empty()) {
            QMessageBox::warning(this, "Error", "API key not configured in preferences.");
            return;
        }

        std::string matlabPath = PreferencesManager::getMatlabPath().toStdString();
        if (matlabPath.empty()) {
            QMessageBox::warning(this, "Error", "MATLAB path not configured in preferences.");
            return;
        }

        // Prepare the API request
        nlohmann::json requestData = {
            {"model", "gpt-4o"},
            {"messages", {{{"role", "user"}, {"content", prompt}}}}
        };
        std::string jsonPayload = requestData.dump();

        // Use setupCurl to initialize CURL
        CURL* curl = nullptr;
        struct curl_slist* headers = nullptr;
        std::string readBuffer;

        matlabutils::setupCurl(
            curl,
            "https://api.openai.com/v1/chat/completions",
            apiKey,
            jsonPayload,
            headers,
            readBuffer
            );

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
        }

        // Use handleApiResponse to process the response and run MATLAB
        std::string result = matlabutils::handleApiResponse(readBuffer, matlabPath);

        // Replace selected text with the evaluated result
        cursor.insertText(QString::fromStdString(result));
        latexEditor->setTextCursor(cursor);

        statusBar()->showMessage("Operation completed successfully.");

        // Cleanup CURL resources
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
        qDebug() << "Error: " << e.what();
        statusBar()->showMessage("Operation failed.");
    }
}

QStringList MainWindow::loadRecentFiles() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString recentFilePath = appDataPath + "/recent.json";

    QFile file(recentFilePath);
    QStringList recentFiles;

    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray array = doc.array();
        for (const QJsonValue &value : array) {
            recentFiles.append(value.toString());
        }
        file.close();
    }

    return recentFiles;
}
void MainWindow::clearRecentFiles() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString recentFilePath = appDataPath + "/recent.json";

    QFile file(recentFilePath);
    if (file.exists()) {
        file.remove(); // Delete the file
    }

    statusBar()->showMessage(tr("Recent files cleared."));
}
void MainWindow::openRecentFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file: %1").arg(filePath));
        return;
    }

    QTextStream in(&file);
    latexEditor->setPlainText(in.readAll());
    file.close();

    currentFile = filePath;
    isModified = false;

    statusBar()->showMessage(tr("Opened file: %1").arg(filePath));
    updateRecentFiles(filePath);
}

void MainWindow::updateRecentFiles(const QString &filePath) {
    if (filePath.isEmpty()) return;

    // Get the writable app data location
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath); // Ensure the directory exists

    QString recentFilePath = appDataPath + "/recent.json";
    QFile file(recentFilePath);

    QJsonArray recentFilesArray;

    // Load existing recent files
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        recentFilesArray = doc.array();
        file.close();
    }

    // Remove duplicates and add the new file path to the start
    QString absolutePath = QFileInfo(filePath).absoluteFilePath();
    for (int i = 0; i < recentFilesArray.size(); ++i) {
        if (recentFilesArray[i].toString() == absolutePath) {
            recentFilesArray.removeAt(i);
            break;
        }
    }
    recentFilesArray.prepend(absolutePath);

    // Save updated list back to file
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument doc(recentFilesArray);
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::toggleMathTextMode() {
    QTextCursor cursor = latexEditor->textCursor();
    QString text = latexEditor->toPlainText();
    int cursorPosition = cursor.position();

    // Check if the cursor is inside a math block
    if (navigation::in_math(text.toStdString(), cursorPosition)) {
        // Insert \text{$1}
        QString insertText = "\\text{$1}";
        cursor.insertText(insertText);
        // Move cursor to the placeholder position (inside {$1})
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, insertText.length() - 3);
    } else {
        // Insert $...$ and place the cursor in the middle
        QString insertText = "$$";
        cursor.insertText(insertText);
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    }

    latexEditor->setTextCursor(cursor);
    qDebug() << "[DEBUG] Math/Text mode toggled.";
}

void MainWindow::processAndInsertText(const QString &text) {
    QTextCursor cursor = latexEditor->textCursor();
    int insertionPosition = cursor.position();

    // Find $1 position in the inserted text
    int placeholder1Index = text.indexOf("$1");

    // Remove all $j patterns
    QString cleanedText = text;
    QRegularExpression dollarPlaceholderRegex(R"(\$\d+)");
    cleanedText.replace(dollarPlaceholderRegex, "");

    // Insert cleaned text
    cursor.insertText(cleanedText);

    // Compute new cursor position
    int cursorTargetPosition = insertionPosition + (placeholder1Index == -1 ? 0 : placeholder1Index);

    // Move cursor to where $1 was
    cursor.setPosition(cursorTargetPosition);
    latexEditor->setTextCursor(cursor);

    // Clear navigation state if any was used
    forwardStack.clear();
    backwardStack.clear();
    statusBar()->showMessage("Inserted and jumped to cursor position.");
}

void MainWindow::onContentsChange(int position, int charsRemoved, int charsAdded) {
    int netChange = charsAdded - charsRemoved;

    // Ensure placeholder navigation is still valid
    // (no explicit placeholder adjustment needed since navigation is regex-based)
    qDebug() << "[DEBUG] onContentsChange:"
             << "position=" << position
             << ", charsRemoved=" << charsRemoved
             << ", charsAdded=" << charsAdded
             << ", netChange=" << netChange;
}
