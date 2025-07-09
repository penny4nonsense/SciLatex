#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <string>
#include <utility> // For std::pair

// Function Declarations
namespace navigation {
    /**
     * Counts valid single and double dollar signs in the LaTeX text.
     * @param latex_text The LaTeX text to analyze.
     * @return A pair of integers: {single dollar count, double dollar count}.
     */
    std::pair<int, int> count_dollar_signs(const std::string& latex_text);

    /**
     * Determines if the cursor is inside a valid math environment.
     * @param text The LaTeX text.
     * @param cursor The cursor position.
     * @return True if inside a math environment; otherwise false.
     */
    bool in_math(const std::string& text, int cursor);

    /**
     * Moves the cursor within a pair of matching delimiters in the specified direction.
     * @param text The text to search.
     * @param cursor The starting position of the cursor.
     * @param left_delim The opening delimiter.
     * @param right_delim The closing delimiter.
     * @param direction 1 for forward, -1 for backward.
     * @return The updated cursor position.
     */
    int move_cursor_within_delimiters(const std::string& text, int cursor, char left_delim, char right_delim, int direction = 1);

    /**
     * Moves the cursor past a \left...\right pair in the specified direction.
     * @param text The LaTeX text.
     * @param cursor The starting position of the cursor.
     * @param direction 1 for forward, -1 for backward.
     * @return The updated cursor position.
     */
    int move_cursor_past_left_right(const std::string& text, int cursor, int direction = 1);

    /**
     * Extracts a substring of specified width centered around the cursor.
     * @param text The input text.
     * @param cursor The cursor position.
     * @param width The width of the window.
     * @return A substring of the specified width.
     */
    std::string window(const std::string& text, int cursor, int width = 10);

    /**
     * Skips spaces to the right of the cursor.
     * @param text The input text.
     * @param cursor The starting position.
     * @return The updated cursor position.
     */
    int ignore_spaces_right(const std::string& text, int cursor);

    /**
     * Skips spaces to the left of the cursor.
     * @param text The input text.
     * @param cursor The starting position.
     * @return The updated cursor position.
     */
    int ignore_spaces_left(const std::string& text, int cursor);

    /**
     * Moves the cursor to the next LaTeX term.
     * @param latex The LaTeX text.
     * @param cursor The starting position of the cursor.
     * @return The updated cursor position.
     */
    int next_term(const std::string& latex, int cursor);

    /**
     * Checks and processes subscripts/superscripts around the cursor.
     * @param latex The LaTeX text.
     * @param cursor The cursor position.
     * @return The updated cursor position.
     */
    int nts_ss_checker(const std::string& latex, int cursor);

    /**
     * Moves the cursor to the next significant term in LaTeX, accounting for special cases.
     * @param latex The LaTeX text.
     * @param cursor The starting position of the cursor.
     * @return The updated cursor position.
     */
    int next_term_special(const std::string& latex, int cursor);

    /**
     * Moves the cursor to the previous LaTeX term.
     * @param latex The LaTeX text.
     * @param cursor The starting position of the cursor.
     * @return The updated cursor position.
     */
    int previous_term(const std::string& latex, int cursor);

    /**
     * Checks and processes subscripts/superscripts around the cursor in reverse.
     * @param latex The LaTeX text.
     * @param cursor The cursor position.
     * @return The updated cursor position.
     */
    int pts_ss_checker(const std::string& latex, int cursor);

    /**
     * Moves the cursor to the previous significant term in LaTeX, accounting for special cases.
     * @param latex The LaTeX text.
     * @param cursor The starting position of the cursor.
     * @return The updated cursor position.
     */
    int previous_term_special(const std::string& latex, int cursor);

}

#endif // NAVIGATION_H
