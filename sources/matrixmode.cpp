
// Include all headers needed
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QStringList>
#include <QList>
#include <stdexcept>

namespace matrixmode{
    // You can keep helpers (like ENV_REGEX and internal static functions) here:
    static const QRegularExpression ENV_REGEX(
        QStringLiteral("\\\\begin\\{(table|array|tabular|matrix|bmatrix|pmatrix|vmatrix|Bmatrix|Vmatrix|smallmatrix|cases|align)\\}(.*?)\\\\end\\{\\1\\}"),
        QRegularExpression::DotMatchesEverythingOption
        );

    QList<QRegularExpressionMatch> findAllEnvironments(const QString &latexContent)
    {
        QList<QRegularExpressionMatch> matchesList;
        QRegularExpressionMatchIterator it = ENV_REGEX.globalMatch(latexContent);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            if (match.hasMatch()) {
                matchesList.append(match);
            }
        }
        return matchesList;
    }

    QRegularExpressionMatch findEnvironmentAtCursor(const QString &latexContent, int cursorPos)
    {
        QList<QRegularExpressionMatch> matches = findAllEnvironments(latexContent);
        if (matches.isEmpty()) {
            throw std::runtime_error("No table or matrix environment found in the document.");
        }

        for (const QRegularExpressionMatch &match : matches) {
            int start = match.capturedStart(0);
            int end = match.capturedEnd(0);
            if (start <= cursorPos && cursorPos <= end) {
                return match;
            }
        }

        throw std::runtime_error("No environment found near the cursor position.");
    }

    QString insertMatrix(const QString &text, int cursor)
    {
        QString insertion = QStringLiteral("\\begin{matrix}\n$1 & $2 \\\\\n$3 & $4\n\\end{matrix}");
        return text.left(cursor) + insertion + text.mid(cursor);
    }

    QString insertTable(const QString &text, int cursor)
    {
        QString insertion = QStringLiteral("\\begin{tabular}{cc}\n$1 & $2 \\\\\n$3 & $4\n\\end{tabular}");
        return text.left(cursor) + insertion + text.mid(cursor);
    }

    // A helper function to extract column specifiers and body for tabular environments.
    struct TabularExtract {
        QString colSpec;
        QString bodyContent;
    };

    static TabularExtract extractTabularContent(const QString &env_content) {
        // Regex: ^\{([^}]*)\}(.*)
        QRegularExpression colSpecPattern(QStringLiteral("^\\{([^}]*)\\}(.*)"),
                                          QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch m = colSpecPattern.match(env_content);
        if (!m.hasMatch()) {
            throw std::runtime_error("Invalid tabular format: Cannot find column specifier.");
        }
        TabularExtract result;
        result.colSpec = m.captured(1);
        result.bodyContent = m.captured(2).trimmed();
        return result;
    }

    // Splits rows by "\\" and returns trimmed non-empty rows
    static QStringList splitRows(const QString &content) {
        // We split by double backslash. In Python the regex was r"\\",
        // Here we literally split by `\\` in the text. Since we are dealing
        // with LaTeX, each row typically ends with `\\`.
        QStringList allParts = content.split("\\", Qt::SkipEmptyParts);
        // Now we must trim and remove empty ones after trim
        QStringList rows;
        for (const QString &part : allParts) {
            QString trimmedPart = part.trimmed();
            if (!trimmedPart.isEmpty()) {
                rows << trimmedPart;
            }
        }
        return rows;
    }

    // Joins rows with ` \\\\` and a newline, as done in Python
    static QString joinRows(const QStringList &rows) {
        // Python code joins with ' \\\\\n', meaning a space, two backslashes, newline.
        // We'll do the same here.
        return rows.join(" \\\\\n");
    }

    // Add a placeholder row or column: '$1', '$2', ...
    static QStringList generatePlaceholders(int count) {
        QStringList placeholders;
        for (int i = 0; i < count; ++i) {
            placeholders << ('$' + QString::number(i + 1));
        }
        return placeholders;
    }

    QString add_row(const QString &latex_content, int row) {
        QList<QRegularExpressionMatch> matches = findAllEnvironments(latex_content);
        if (matches.isEmpty()) {
            throw std::runtime_error("No table or matrix environment found in the document.");
        }

        QRegularExpressionMatch match = matches.first();
        QString env_name = match.captured(1);
        QString full_env = match.captured(0);
        int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        QString env_content = match.captured(2);

        QString modified_env;
        if (env_name == QLatin1String("tabular")) {
            TabularExtract extracted = extractTabularContent(env_content);
            QStringList rows = splitRows(extracted.bodyContent);

            int cols = (rows.isEmpty()) ? 0 : rows.first().split("&").size();
            if (row < 0 || row > rows.size()) {
                throw std::out_of_range("Row index out of range for table.");
            }

            QString new_row = generatePlaceholders(cols).join(" & ");
            rows.insert(row, new_row);

            QString newBody = joinRows(rows);
            modified_env = QString("\\begin{%1}{%2}\n%3\n\\end{%1}")
                               .arg(env_name, extracted.colSpec, newBody);

        } else {
            // Matrix-like environments
            QStringList rows = splitRows(env_content);
            int cols = (rows.isEmpty()) ? 2 : rows.first().split("&").size();
            // Default to 2 if empty

            if (row < 0 || row > rows.size()) {
                throw std::out_of_range("Row index out of range for matrix.");
            }

            QString new_row = generatePlaceholders(cols).join(" & ");
            rows.insert(row, new_row);

            QString newBody = joinRows(rows);
            modified_env = QString("\\begin{%1}\n%2\n\\end{%1}").arg(env_name, newBody);
        }

        return latex_content.left(start) + modified_env + latex_content.mid(end);
    }

    QString delete_row(const QString &latex_content, int row) {
        QList<QRegularExpressionMatch> matches = findAllEnvironments(latex_content);
        if (matches.isEmpty()) {
            throw std::runtime_error("No table or matrix environment found in the document.");
        }

        QRegularExpressionMatch match = matches.first();
        QString env_name = match.captured(1);
        int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        QString env_content = match.captured(2);

        QString modified_env;
        if (env_name == QLatin1String("tabular")) {
            TabularExtract extracted = extractTabularContent(env_content);
            QStringList rows = splitRows(extracted.bodyContent);

            if (row < 0 || row >= rows.size()) {
                throw std::out_of_range("Row index out of range for table.");
            }

            rows.removeAt(row);

            QString newBody = joinRows(rows);
            modified_env = QString("\\begin{%1}{%2}\n%3\n\\end{%1}")
                               .arg(env_name, extracted.colSpec, newBody);

        } else {
            // Matrix-like
            QStringList rows = splitRows(env_content);

            if (row < 0 || row >= rows.size()) {
                throw std::out_of_range("Row index out of range for matrix.");
            }

            rows.removeAt(row);

            QString newBody = joinRows(rows);
            modified_env = QString("\\begin{%1}\n%2\n\\end{%1}").arg(env_name, newBody);
        }

        return latex_content.left(start) + modified_env + latex_content.mid(end);
    }

    QString add_column(const QString &latex_content, int col) {
        QList<QRegularExpressionMatch> matches = findAllEnvironments(latex_content);
        if (matches.isEmpty()) {
            throw std::runtime_error("No table or matrix environment found in the document.");
        }

        QRegularExpressionMatch match = matches.first();
        QString env_name = match.captured(1);
        int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        QString env_content = match.captured(2).trimmed();

        QString modified_env;
        // Common logic to split out rows for both tabular and matrix
        auto splitRows = [](const QString &content) {
            QStringList allParts = content.split("\\", Qt::SkipEmptyParts);
            QStringList rows;
            for (const QString &part : allParts) {
                QString trimmedPart = part.trimmed();
                if (!trimmedPart.isEmpty()) {
                    rows << trimmedPart;
                }
            }
            return rows;
        };

        if (env_name == QLatin1String("tabular")) {
            // Extract column spec and body
            QRegularExpression colSpecPattern(QStringLiteral("^\\{([^}]*)\\}(.*)"),
                                              QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch csm = colSpecPattern.match(env_content);
            if (!csm.hasMatch()) {
                throw std::runtime_error("Invalid tabular format: Cannot find column specifier.");
            }

            QString col_spec = csm.captured(1);
            QString body_content = csm.captured(2).trimmed();
            QStringList rows = splitRows(body_content);

            int current_cols = 0;
            if (!rows.isEmpty()) {
                current_cols = rows.first().split("&").size();
            } else {
                // If no rows, count columns from col_spec
                for (QChar ch : col_spec) {
                    if (ch == 'l' || ch == 'c' || ch == 'r') {
                        current_cols++;
                    }
                }
            }

            if (col < 0 || col > current_cols) {
                throw std::out_of_range("Column index out of range for the given table.");
            }

            // Insert 'c' in col_spec at the right place
            QList<int> c_indices;
            for (int i = 0; i < col_spec.size(); ++i) {
                QChar ch = col_spec.at(i);
                if (ch == 'l' || ch == 'c' || ch == 'r') {
                    c_indices.append(i);
                }
            }

            QString new_col_spec;
            if (c_indices.isEmpty()) {
                // no existing columns, just append 'c'
                new_col_spec = col_spec + 'c';
            } else {
                // Insert 'c' at position corresponding to 'col'
                int insertPos;
                if (col == c_indices.size()) {
                    insertPos = c_indices.last() + 1;
                } else {
                    insertPos = c_indices[col];
                }
                new_col_spec = col_spec;
                new_col_spec.insert(insertPos, 'c');
            }

            // Now assign placeholders based on row number:
            // Row 0 -> $1, Row 1 -> $2, Row 2 -> $3, etc.
            for (int i = 0; i < rows.size(); ++i) {
                QStringList columns = rows[i].split("&");
                for (int j = 0; j < columns.size(); ++j) {
                    columns[j] = columns[j].trimmed();
                }
                columns.insert(col, '$' + QString::number(i + 1));
                rows[i] = columns.join(" & ");
            }

            if (rows.isEmpty()) {
                // If no rows, create one row with a single placeholder for column
                rows << "$1";
            }

            QString modified_body = rows.join(" \\\\\n");
            modified_env = QString("\\begin{%1}{%2}\n%3\n\\end{%1}")
                               .arg(env_name, new_col_spec, modified_body);

        } else {
            // Matrix-like environments
            QStringList rows = splitRows(env_content);
            int current_cols = (!rows.isEmpty()) ? rows.first().split("&").size() : 0;

            if (col < 0 || col > current_cols) {
                throw std::out_of_range("Column index out of range for the given matrix.");
            }

            for (int i = 0; i < rows.size(); ++i) {
                QStringList columns = rows[i].split("&");
                for (int j = 0; j < columns.size(); ++j) {
                    columns[j] = columns[j].trimmed();
                }
                // Row i gets placeholder '$i+1'
                columns.insert(col, '$' + QString::number(i + 1));
                rows[i] = columns.join(" & ");
            }

            if (rows.isEmpty()) {
                // If no rows, create one row with a single placeholder
                rows << "$1";
            }

            QString modified_body = rows.join(" \\\\\n");
            modified_env = QString("\\begin{%1}\n%2\n\\end{%1}").arg(env_name, modified_body);
        }

        return latex_content.left(start) + modified_env + latex_content.mid(end);
    }

    QString delete_column(const QString &latex_content, int col) {
        QList<QRegularExpressionMatch> matches = findAllEnvironments(latex_content);
        if (matches.isEmpty()) {
            throw std::runtime_error("No table or matrix environment found in the document.");
        }

        QRegularExpressionMatch match = matches.first();
        QString env_name = match.captured(1);
        int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        QString env_content = match.captured(2).trimmed();

        QString modified_env;
        if (env_name == QLatin1String("tabular")) {
            TabularExtract extracted = extractTabularContent(env_content);
            QStringList rows = splitRows(extracted.bodyContent);

            if (rows.isEmpty()) {
                throw std::runtime_error("No columns to delete in an empty table.");
            }

            int current_cols = rows.first().split("&").size();
            if (col < 0 || col >= current_cols) {
                throw std::out_of_range("Column index out of range for table.");
            }

            // Identify column spec indices
            QList<int> c_indices;
            for (int i = 0; i < extracted.colSpec.size(); ++i) {
                QChar ch = extracted.colSpec.at(i);
                if (ch == 'l' || ch == 'c' || ch == 'r') {
                    c_indices.append(i);
                }
            }

            if (c_indices.size() <= col) {
                throw std::runtime_error("Mismatch in column specs and actual columns.");
            }

            // Remove the column spec at c_indices[col]
            QString new_col_spec = extracted.colSpec;
            new_col_spec.remove(c_indices[col], 1);

            // Remove the column from each row
            QStringList modified_rows;
            for (QString r : rows) {
                QStringList columns = r.split("&");
                for (int i = 0; i < columns.size(); ++i)
                    columns[i] = columns[i].trimmed();

                columns.removeAt(col);
                modified_rows << columns.join(" & ");
            }

            QString newBody = joinRows(modified_rows);
            modified_env = QString("\\begin{%1}{%2}\n%3\n\\end{%1}")
                               .arg(env_name, new_col_spec, newBody);

        } else {
            // Matrix environment
            QStringList rows = splitRows(env_content);

            if (rows.isEmpty()) {
                throw std::runtime_error("No columns to delete in an empty matrix.");
            }

            int current_cols = rows.first().split("&").size();
            if (col < 0 || col >= current_cols) {
                throw std::out_of_range("Column index out of range for matrix.");
            }

            QStringList modified_rows;
            for (QString r : rows) {
                QStringList columns = r.split("&");
                for (int i = 0; i < columns.size(); ++i)
                    columns[i] = columns[i].trimmed();

                columns.removeAt(col);
                modified_rows << columns.join(" & ");
            }

            QString newBody = joinRows(modified_rows);
            modified_env = QString("\\begin{%1}\n%2\n\\end{%1}").arg(env_name, newBody);
        }

        return latex_content.left(start) + modified_env + latex_content.mid(end);
    }

    std::pair<int,int> find_latex_table_cursor_position(const QString &latex_string, int cursor_position) {
        QRegularExpressionMatch match;
        try {
            match = findEnvironmentAtCursor(latex_string, cursor_position);
        } catch (const std::exception &e) {
            // If environment isn't found, handle the error similarly to Python code
            throw std::runtime_error("No environment found at given cursor position.");
        }

        int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        QString content = match.captured(2); // The inner environment content
        QString env_name = match.captured(1);

        // Calculate relative_cursor_position:
        // Python: relative_cursor_position = cursor_position - (start + len("\\begin{"+env_name+"}"))
        // Count length of "\begin{" + env_name + "}".
        // "\begin{" is 7 chars: '\\'(1) 'b'(2) 'e'(3) 'g'(4) 'i'(5) 'n'(6) '{'(7)
        // Add env_name.size(), and add 1 char for '}'
        int begin_env_len = 7 + env_name.size() + 1; // == 8 + env_name.size() but counting explicitly
        int relative_cursor_position = cursor_position - (start + begin_env_len);

        // Split rows by '\\'
        // Python code does: rows = content.split("\\\\")
        // We'll do the same with Qt. Note: We need a literal "\\" for backslash.
        // Use a simple split on "\\\\". In LaTeX, rows typically end with "\\", so splitting on "\\\\" means splitting by double-backslash.
        // If we need to split literally on `\\`, we must note that in Python `content.split("\\\\")` means splitting on two literal backslashes.
        // In C++, splitting on "\\\\" also means splitting on two backslashes.
        QStringList rows = content.split("\\\\", Qt::KeepEmptyParts);

        int cumulative_length = 0;
        for (int row_index = 0; row_index < rows.size(); ++row_index) {
            const QString &row_text = rows[row_index];
            int row_len = row_text.length();
            // Check if cursor is within this row
            if (relative_cursor_position <= (cumulative_length + row_len)) {
                // Cursor is in this row.
                // Split by '&'
                QStringList columns = row_text.split("&", Qt::KeepEmptyParts);

                int col_start = cumulative_length;
                for (int col_index = 0; col_index < columns.size(); ++col_index) {
                    const QString &col_text = columns[col_index];
                    int col_end = col_start + col_text.length();
                    if (col_start <= relative_cursor_position && relative_cursor_position <= col_end) {
                        return std::make_pair(row_index, col_index);
                    }
                    col_start = col_end + 1; // +1 for the '&' separator
                }
                // If no column matched exactly, assume end of row in last column
                return std::make_pair(row_index, columns.size() - 1);
            }

            // Move to next row: add row_len plus length of the row separator "\\\\"
            cumulative_length += row_len + 2; // "\\\\" has length 2
        }

        // If no row matched, assume last row, last column:
        if (rows.isEmpty()) {
            return std::make_pair(0, 0);
        } else {
            QStringList lastColumns = rows.last().split("&", Qt::KeepEmptyParts);
            return std::make_pair(rows.size() - 1, lastColumns.size() - 1);
        }
    }

    std::pair<QString,int> extract_latex_table_and_relative_cursor(const QString &latex_string, int cursor_position) {
        QRegularExpressionMatch match;
        try {
            match = findEnvironmentAtCursor(latex_string, cursor_position);
        } catch (const std::exception &e) {
            throw std::runtime_error("No environment found at given cursor position.");
        }

        int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        QString full_env = match.captured(0);

        int relative_cursor_position = cursor_position - start;

        return std::make_pair(full_env, relative_cursor_position);
    }

    static int sumRowLengths(const QStringList &rows, int upToRow) {
        // sum(len(r) + len("\\\\")) for rows[:upToRow] in Python
        // Here "\\\\" is 2 characters
        int total = 0;
        for (int i = 0; i < upToRow && i < rows.size(); ++i) {
            total += rows[i].length() + 2; // each row ends with "\\"
        }
        return total;
    }

    static int sumColumnsLengths(const QStringList &columns, int upToCol) {
        // sum(len(columns[j]) + len("&")) for columns[:upToCol]
        // "&" length is 1
        int total = 0;
        for (int j = 0; j < upToCol && j < columns.size(); ++j) {
            total += columns[j].length();
            if (j < upToCol) {
                total += 1; // for the '&' after the column
            }
        }
        return total;
    }

    std::pair<QString,int> insert_row_before_cursor(const QString &latex_string, int cursor_position) {
        auto [table, relative_cursor] = extract_latex_table_and_relative_cursor(latex_string, cursor_position);
        auto [row, _col] = find_latex_table_cursor_position(table, relative_cursor);

        QString modified_table = add_row(table, row);

        int start = latex_string.indexOf(table);
        if (start == -1) {
            throw std::runtime_error("Table not found in latex_string.");
        }
        int end = start + table.length();

        QString modified_latex = latex_string.left(start) + modified_table + latex_string.mid(end);

        // new_cursor_position = cursor_position + (len(modified_table) - len(table))
        int new_cursor_position = cursor_position + (modified_table.length() - table.length());

        return std::make_pair(modified_latex, new_cursor_position);
    }

    std::pair<QString,int> insert_row_after_cursor(const QString &latex_string, int cursor_position) {
        auto [table, relative_cursor] = extract_latex_table_and_relative_cursor(latex_string, cursor_position);
        auto [row, _col] = find_latex_table_cursor_position(table, relative_cursor);

        QString modified_table = add_row(table, row + 1);

        int start = latex_string.indexOf(table);
        if (start == -1) {
            throw std::runtime_error("Table not found in latex_string.");
        }
        int end = start + table.length();

        QString modified_latex = latex_string.left(start) + modified_table + latex_string.mid(end);

        // new_cursor_position = cursor_position (no additional change mentioned)
        int new_cursor_position = cursor_position;

        return std::make_pair(modified_latex, new_cursor_position);
    }

    QString insert_column_before_cursor(const QString &latex_string, int cursor_position) {
        auto [table, relative_cursor] = extract_latex_table_and_relative_cursor(latex_string, cursor_position);
        auto [_row, column] = find_latex_table_cursor_position(table, relative_cursor);

        QString modified_table = add_column(table, column);

        int start = latex_string.indexOf(table);
        if (start == -1) {
            throw std::runtime_error("Table not found in latex_string.");
        }
        int end = start + table.length();

        QString modified_latex = latex_string.left(start) + modified_table + latex_string.mid(end);

        return modified_latex;
    }

    QString insert_column_after_cursor(const QString &latex_string, int cursor_position) {
        auto [table, relative_cursor] = extract_latex_table_and_relative_cursor(latex_string, cursor_position);
        auto [_row, column] = find_latex_table_cursor_position(table, relative_cursor);

        QString modified_table = add_column(table, column + 1);

        int start = latex_string.indexOf(table);
        if (start == -1) {
            throw std::runtime_error("Table not found in latex_string.");
        }
        int end = start + table.length();

        QString modified_latex = latex_string.left(start) + modified_table + latex_string.mid(end);

        return modified_latex;
    }

    std::pair<QString,int> delete_row_at_cursor(const QString &latex_string, int cursor_position) {
        auto [table, relative_cursor] = extract_latex_table_and_relative_cursor(latex_string, cursor_position);
        auto [row, _col] = find_latex_table_cursor_position(table, relative_cursor);

        QString modified_table = delete_row(table, row);

        // Adjust the cursor position:
        QStringList rows = table.split("\\\\", Qt::KeepEmptyParts);
        int new_relative_cursor;
        if (row < rows.size() - 1) {
            // Move to the start of the next row
            new_relative_cursor = sumRowLengths(rows, row);
        } else {
            // Move to the start of the previous row
            new_relative_cursor = sumRowLengths(rows, row - 1);
        }

        int start = latex_string.indexOf(table);
        if (start == -1) {
            throw std::runtime_error("Table not found in latex_string.");
        }
        int end = start + table.length();
        QString modified_latex = latex_string.left(start) + modified_table + latex_string.mid(end);

        int new_cursor_position = start + new_relative_cursor;

        return std::make_pair(modified_latex, new_cursor_position);
    }

    std::pair<QString,int> delete_column_at_cursor(const QString &latex_string, int cursor_position) {
        auto [table, relative_cursor] = extract_latex_table_and_relative_cursor(latex_string, cursor_position);
        auto [_row, column] = find_latex_table_cursor_position(table, relative_cursor);

        QString modified_table = delete_column(table, column);

        // Adjust the cursor position:
        // Iterate over rows to find where to place the cursor.
        QStringList rows = table.split("\\\\", Qt::KeepEmptyParts);
        int cumulative_length = 0;
        // The logic:
        // If the column was not the last column, move to start of the next column in same row
        // Otherwise move to the last column in that row.

        // We must replicate the Python logic:
        // The Python code:
        // for row in rows:
        //   columns = row.split("&")
        //   if column < len(columns)-1:
        //       cumulative_length += sum of lengths up to column (not inclusive)
        //       break
        //   else:
        //       move to the last column in the row

        // Actually, Python code seems incomplete or ambiguous. We'll follow it exactly:
        {
            bool done = false;
            for (const QString &r : rows) {
                QStringList columns = r.split("&", Qt::KeepEmptyParts);
                if (column < columns.size() - 1) {
                    // Move to the start of the next column in the same row:
                    // sum(len(columns[j])+len("&")) for j in range(column)
                    // means position at column-th column start
                    cumulative_length += sumColumnsLengths(columns, column);
                    done = true;
                    break;
                } else {
                    // Move to the last column in the row:
                    // sum(len(columns[j])+len("&")) for j in range(len(columns)-1)
                    cumulative_length += sumColumnsLengths(columns, columns.size() - 1);
                }
            }
            // If done is false, means we ended at last row. cumulative_length points at last column of last row.
        }

        int start = latex_string.indexOf(table);
        if (start == -1) {
            throw std::runtime_error("Table not found in latex_string.");
        }
        int end = start + table.length();
        QString modified_latex = latex_string.left(start) + modified_table + latex_string.mid(end);

        int new_cursor_position = start + cumulative_length;

        return std::make_pair(modified_latex, new_cursor_position);
    }
}
