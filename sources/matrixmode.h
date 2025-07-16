#ifndef MATRIXMODE_H
#define MATRIXMODE_H

#include <QString>
#include <QRegularExpressionMatch>
#include <utility>
#include <stdexcept>
#include <QList>

// If you need forward declarations of Qt classes, you can also do:
// class QString;
// class QRegularExpressionMatch;

namespace matrixmode {
    // Struct declarations
    struct TabularExtract {
        QString colSpec;
        QString bodyContent;
    };

    // Function declarations (only the public interface you want to use):
    QList<QRegularExpressionMatch> findAllEnvironments(const QString &latexContent);
    QRegularExpressionMatch findEnvironmentAtCursor(const QString &latexContent, int cursorPos);

    QString insertMatrix(const QString &text, int cursor);
    QString insertTable(const QString &text, int cursor);

    QString add_row(const QString &latex_content, int row);
    QString delete_row(const QString &latex_content, int row);
    QString add_column(const QString &latex_content, int col);
    QString delete_column(const QString &latex_content, int col);

    std::pair<int,int> find_latex_table_cursor_position(const QString &latex_string, int cursor_position);
    std::pair<QString,int> extract_latex_table_and_relative_cursor(const QString &latex_string, int cursor_position);

    std::pair<QString,int> insert_row_before_cursor(const QString &latex_string, int cursor_position);
    std::pair<QString,int> insert_row_after_cursor(const QString &latex_string, int cursor_position);

    QString insert_column_before_cursor(const QString &latex_string, int cursor_position);
    QString insert_column_after_cursor(const QString &latex_string, int cursor_position);

    std::pair<QString,int> delete_row_at_cursor(const QString &latex_string, int cursor_position);
    std::pair<QString,int> delete_column_at_cursor(const QString &latex_string, int cursor_position);

}

#endif // MATRIXMODE_H
