#include "navigation.h"
#include <regex>
#include <stdexcept> // For std::runtime_error and std::invalid_argument
#include <unordered_set>
#include <algorithm> // For std::max and std::min

namespace navigation {
    std::pair<int, int> count_dollar_signs(const std::string& latex_text) {
        // Step 1: Define the regex pattern for ignored environments
        std::regex ignored_pattern(R"(\\begin\{(array|matrix|bmatrix|pmatrix|vmatrix|tabular)\*?\}(?:.|[\n])*?\\end\{\1\*?\})");

        // Function to count single and double dollar signs
        auto count_dollars = [](const std::string& text) -> std::pair<int, int> {
            int single_dollars = 0;
            int double_dollars = 0;
            size_t i = 0;

            while (i < text.length()) {
                if (text[i] == '$') {
                    // Check for double dollar ($$)
                    if (i + 1 < text.length() && text[i + 1] == '$') {
                        double_dollars++;
                        i += 2; // Skip the second $
                        continue;
                    }
                    // Count single dollar
                    single_dollars++;
                }
                i++;
            }

            return {single_dollars, double_dollars};
        };

        // Step 1: Remove ignored environments
        std::string cleaned_text = std::regex_replace(latex_text, ignored_pattern, "");

        // Step 2: Remove escaped dollar signs
        cleaned_text = std::regex_replace(cleaned_text, std::regex(R"(\\\$)"), "");

        // Step 3: Count valid single and double dollar signs
        return count_dollars(cleaned_text);
    }

    bool in_math(const std::string& text, int cursor) {
        // Define regex for math environments
        std::regex begin_pattern(R"(\\begin\{(equation|eqnarray|multline|align|gather|flalign|array|displaymath|split)\*?\})");
        std::regex end_pattern(R"(\\end\{(equation|eqnarray|multline|align|gather|flalign|array|displaymath|split)\*?\})");
        std::regex begin_bracket(R"(\\\[)");
        std::regex end_bracket(R"(\\\])");

        // Helper function to check if inside an environment
        auto inside_environment = [&](const std::string& text, int cursor,
                                      const std::regex& begin_pattern, const std::regex& end_pattern) -> bool {
            std::smatch match;
            int last_begin = -1;
            int first_end = -1;

            // Find the last \begin before the cursor
            for (std::sregex_iterator it(text.begin(), text.end(), begin_pattern), end; it != end; ++it) {
                if (it->position() < cursor) {
                    last_begin = it->position();
                } else {
                    break;
                }
            }

            // Find the first \end after the cursor
            for (std::sregex_iterator it(text.begin(), text.end(), end_pattern), end; it != end; ++it) {
                if (it->position() > cursor) {
                    first_end = it->position();
                    break;
                }
            }

            // Check if cursor is surrounded by a valid environment
            return last_begin != -1 && first_end != -1 && last_begin < cursor && cursor < first_end;
        };

        // Check environments with \begin{}...\end{}
        bool in_block_env = inside_environment(text, cursor, begin_pattern, end_pattern);

        // Check environments with \[...\]
        bool in_bracket_env = inside_environment(text, cursor, begin_bracket, end_bracket);

        // Check dollar signs
        auto [lower_single, lower_double] = count_dollar_signs(text.substr(0, cursor));
        auto [upper_single, upper_double] = count_dollar_signs(text.substr(cursor));

        bool single_valid = (lower_single % 2 == upper_single % 2);
        bool double_valid = (lower_double % 2 == upper_double % 2);

        // If either dollar environment is invalid, return false
        if (!single_valid || !double_valid) {
            return false;
        }

        // Return true if in any valid math environment
        if (in_block_env || in_bracket_env) {
            return true;
        }
        if (lower_single % 2 == 1 || lower_double % 2 == 1) {
            return true;
        }

        // Not in a math environment
        return false;
    }

    int move_cursor_within_delimiters(const std::string& text, int cursor, char left_delim, char right_delim, int direction) {
        int n = static_cast<int>(text.length());

        // Validate direction
        if (direction != 1 && direction != -1) {
            throw std::invalid_argument("Invalid direction: must be 1 or -1");
        }

        // Check if cursor is at the appropriate delimiter
        bool at_delimiter = (direction == 1 && cursor < n && text[cursor] == left_delim) ||
                            (direction == -1 && cursor >= 0 && text[cursor] == right_delim);

        if (at_delimiter) {
            cursor += direction;
            int depth = 1;

            // Traverse the text while ensuring bounds and depth constraints
            while (cursor >= 0 && cursor < n && depth > 0) {
                if (text[cursor] == left_delim) {
                    depth += direction;
                } else if (text[cursor] == right_delim) {
                    depth -= direction;
                }
                cursor += direction;
            }

            // If depth is not zero, the matching delimiter wasn't found
            if (depth != 0) {
                throw std::runtime_error("Matching delimiter not found");
            }

            return cursor;
        }

        // Return the original cursor if not at a valid starting delimiter
        return cursor;
    }


    int move_cursor_past_left_right(const std::string& text, int cursor, int direction) {
        // Define regex patterns for left and right delimiters
        std::regex right_command_pattern(direction == 1
                                             ? R"(^\\right\\[a-zA-Z]+|^\\right[)\]\}\.])"
                                             : R"(\\right\\[a-zA-Z]+$|\\right[)\]\}]$)");
        std::regex left_command_pattern(direction == 1
                                            ? R"(^\\left\\[a-zA-Z]+|^\\left[(\[\{\.])"
                                            : R"(\\left\\[a-zA-Z]+$|\\left[(\[\{]$)");

        int n = static_cast<int>(text.length());

        if (direction != 1 && direction != -1) {
            throw std::invalid_argument("Invalid direction: must be 1 or -1");
        }

        std::smatch match;

        if (direction == 1) {
            // Moving forward
            std::string remaining_text = text.substr(cursor);
            if (std::regex_search(remaining_text, match, left_command_pattern)) {
                cursor += static_cast<int>(match[0].length());
                int depth = 1;

                while (cursor < n) {
                    bool flag = true;
                    remaining_text = text.substr(cursor);

                    if (std::regex_search(remaining_text, match, left_command_pattern)) {
                        cursor += static_cast<int>(match[0].length());
                        depth += 1;
                        flag = false;
                    } else if (std::regex_search(remaining_text, match, right_command_pattern)) {
                        cursor += static_cast<int>(match[0].length());
                        depth -= 1;
                        flag = false;
                    }

                    if (depth == 0) {
                        break;
                    }
                    if (flag) {
                        cursor += 1;
                    }
                }
            }
        } else {
            // Moving backward
            std::string processed_text = text.substr(0, cursor + 1);
            if (std::regex_search(processed_text, match, right_command_pattern)) {
                cursor -= static_cast<int>(match[0].length());
                int depth = 1;

                while (cursor > 0) {
                    bool flag = true;
                    processed_text = text.substr(0, cursor + 1);

                    if (std::regex_search(processed_text, match, left_command_pattern)) {
                        cursor -= static_cast<int>(match[0].length());
                        depth -= 1;
                        flag = false;
                    } else if (std::regex_search(processed_text, match, right_command_pattern)) {
                        cursor -= static_cast<int>(match[0].length());
                        depth += 1;
                        flag = false;
                    }

                    if (depth == 0) {
                        break;
                    }
                    if (flag) {
                        cursor -= 1;
                    }
                }
            }
        }

        return cursor;
    }



    // Function definition
    std::string window(const std::string& text, int cursor, int width) {
        int half = width / 2;
        int start = std::max(0, cursor - half);
        int end = std::min(static_cast<int>(text.length()), cursor + half);
        return text.substr(start, end - start);
    }

    int ignore_spaces_right(const std::string& text, int cursor) {
        int n = static_cast<int>(text.length());
        while (cursor < n && text[cursor] == ' ') {
            cursor++;
        }
        return (cursor >= n) ? n : cursor;
    }

    int ignore_spaces_left(const std::string& text, int cursor) {
        cursor = std::min(cursor, static_cast<int>(text.length()) - 1);
        while (cursor > 0 && text[cursor] == ' ') {
            cursor--;
        }
        return (cursor <= 0) ? 0 : cursor;
    }

    int next_term(const std::string& latex, int cursor) {
        int n = static_cast<int>(latex.length());
        std::regex pattern(R"(\\[a-zA-Z]+)"); // Matches LaTeX commands like \command

        if (cursor >= n) {
            return n; // End of input
        }

        // Skip spaces
        cursor = ignore_spaces_right(latex, cursor);

        // Handle LaTeX commands (\command)
        if (cursor < n && latex[cursor] == '\\') {
            std::smatch match;
            std::string remaining_text = latex.substr(cursor);
            if (std::regex_search(remaining_text, match, pattern) && match.position() == 0) {
                return cursor + static_cast<int>(match[0].length());
            }
        }

        // Skip spaces
        cursor = ignore_spaces_right(latex, cursor);

        // Handle superscripts (^...) and subscripts (_...)
        if (cursor < n && (latex[cursor] == '^' || latex[cursor] == '_')) {
            if (cursor + 1 < n && latex[cursor + 1] == '{') {
                cursor += 2; // Move past the ^{ or _{
                return cursor;
            }
        }

        // Regular characters or other symbols
        if (cursor < n) {
            cursor += 1;
        }

        // Skip spaces
        cursor = ignore_spaces_right(latex, cursor);

        return cursor;
    }

    int nts_ss_checker(const std::string& latex, int cursor) {
        int previous_cursor = -1;
        int n = static_cast<int>(latex.length());

        while (previous_cursor != cursor) {
            previous_cursor = cursor;

            if (cursor >= n) return n;

            if (latex[cursor] == '^') {
                cursor += 1;
                int prev_cursor = cursor;
                cursor = move_cursor_within_delimiters(latex, cursor, '{', '}');
                if (cursor == prev_cursor) {
                    cursor += 1;
                }
                cursor = ignore_spaces_right(latex, cursor);
            }

            if (cursor >= n) return n;

            if (latex[cursor] == '_') {
                cursor += 1;
                int prev_cursor = cursor;
                cursor = move_cursor_within_delimiters(latex, cursor, '{', '}');
                if (cursor == prev_cursor) {
                    cursor += 1;
                }
                cursor = ignore_spaces_right(latex, cursor);
            }

            if (cursor >= n) return n;

            if (latex[cursor] == '\'') {
                cursor += 1;
                cursor = ignore_spaces_right(latex, cursor);
            }
        }

        return cursor;
    }

    int next_term_special(const std::string& latex, int cursor) {
        int n = static_cast<int>(latex.length());

        std::regex latex_pattern(R"(^\\[a-zA-Z]+)");
        std::regex number_pattern(R"(^(-?\d{1,3}(,\d{3})*(\.\d+)?|\d+(\.\d+)?))");
        std::regex left_command_pattern(R"(^\\left(?:\\[a-zA-Z]+|[\[\(\.\|\{\}<>]))");

        if (cursor >= n) return n;

        // Skip spaces
        cursor = ignore_spaces_right(latex, cursor);

        std::smatch match;

        // Get remaining string from the cursor
        std::string remaining_text = latex.substr(cursor);

        if (std::regex_search(remaining_text, match, left_command_pattern)) {
            cursor = move_cursor_past_left_right(latex, cursor);
        } else if (std::regex_search(remaining_text, match, number_pattern)) {
            cursor += static_cast<int>(match[0].length());
        } else if (std::regex_search(remaining_text, match, latex_pattern)) {
            std::string command_name = match[0].str().substr(1);
            cursor += static_cast<int>(match[0].length());
            cursor = ignore_spaces_right(latex, cursor);

            std::unordered_set<std::string> two_arg_commands = {"frac"};
            std::unordered_set<std::string> two_arg_commands_amp = {"overset", "underset"};

            if (two_arg_commands.count(command_name)) {
                if (cursor < n && latex[cursor] == '{') {
                    cursor = move_cursor_within_delimiters(latex, cursor, '{', '}');
                }
                cursor = ignore_spaces_right(latex, cursor);
                if (cursor < n && latex[cursor] == '{') {
                    cursor = move_cursor_within_delimiters(latex, cursor, '{', '}');
                }
            } else if (two_arg_commands_amp.count(command_name)) {
                if (cursor < n && latex[cursor] == '{') {
                    cursor = move_cursor_within_delimiters(latex, cursor, '{', '}');
                }
                cursor = ignore_spaces_right(latex, cursor);
                if (cursor < n && latex[cursor] == '&') {
                    cursor += 1; // Skip the '&'
                }
                cursor = ignore_spaces_right(latex, cursor);
                if (cursor < n && latex[cursor] == '{') {
                    cursor = move_cursor_within_delimiters(latex, cursor, '{', '}');
                }
            } else {
                if (cursor < n && latex[cursor] == '[') {
                    cursor = move_cursor_within_delimiters(latex, cursor, '[', ']');
                }
                cursor = ignore_spaces_right(latex, cursor);
                if (cursor < n && latex[cursor] == '{') {
                    cursor = move_cursor_within_delimiters(latex, cursor, '{', '}');
                }
            }
        } else {
            cursor += 1;
        }

        // Skip spaces
        cursor = ignore_spaces_right(latex, cursor);
        cursor = nts_ss_checker(latex, cursor);

        return cursor;
    }

    int previous_term(const std::string& latex, int cursor) {
        std::regex pattern(R"(.*\\[a-zA-Z]+$)"); // Matches a LaTeX command ending at the cursor

        if (cursor <= 0) {
            return 0; // Beginning of input
        }

        // Skip spaces
        cursor = ignore_spaces_left(latex, cursor);

        // Handle LaTeX commands (\command)
        std::string substr = latex.substr(0, cursor);
        std::smatch match;
        if (std::regex_match(substr, match, pattern)) {
            size_t backslash_pos = substr.rfind('\\');
            return static_cast<int>(backslash_pos);
        }

        // Skip spaces
        cursor = ignore_spaces_left(latex, cursor);

        // Handle superscripts (^...) and subscripts (_...)
        if (cursor - 2 >= 0 && (latex[cursor - 2] == '^' || latex[cursor - 2] == '_') && latex[cursor - 1] == '{') {
            cursor -= 2; // Move past the ^{ or _{
            return cursor;
        }

        // Regular characters or other symbols
        cursor -= 1;

        // Skip spaces
        cursor = ignore_spaces_left(latex, cursor);

        return cursor;
    }

    int pts_ss_checker(const std::string& latex, int cursor) {
        std::regex subscript_pattern(R"(_\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}$)");
        std::regex superscript_pattern(R"(\^\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}$)");
        std::regex simple_subscript_pattern(R"(_\w+$)");
        std::regex simple_superscript_pattern(R"(\^\w+$)");

        int previous_cursor = -1;

        while (previous_cursor != cursor) {
            previous_cursor = cursor;

            if (cursor <= 0) {
                return 0;
            }

            std::smatch match;
            std::string remaining_text = latex.substr(0, cursor);

            if (std::regex_search(remaining_text, match, subscript_pattern)) {
                cursor -= static_cast<int>(match[0].length());
            }
            if (std::regex_search(remaining_text, match, superscript_pattern)) {
                cursor -= static_cast<int>(match[0].length());
            }
            if (std::regex_search(remaining_text, match, simple_subscript_pattern)) {
                cursor -= static_cast<int>(match[0].length());
            }
            if (std::regex_search(remaining_text, match, simple_superscript_pattern)) {
                cursor -= static_cast<int>(match[0].length());
            }

            cursor = ignore_spaces_left(latex, cursor);

            if (cursor <= 0) {
                return 0;
            }
        }

        return cursor;
    }

    // Main function: previous_term_special
    int previous_term_special(const std::string& latex, int cursor) {
        std::regex latex_pattern(R"(\\[a-zA-Z]+$)");
        std::regex number_pattern(R"([-]?\b\d+(\.\d+)?$)");
        std::regex right_command_pattern(R"(\\right(?:\\[a-zA-Z]+|[\]\)\.\|\}<>])$)");

        if (cursor <= 0) {
            return 0; // End of input
        }

        // Skip spaces
        cursor = ignore_spaces_left(latex, cursor);

        // Handle single quote
        if (cursor > 0 && latex[cursor - 1] == '\'') {
            cursor -= 1;
        }

        cursor = ignore_spaces_left(latex, cursor);

        // Process subscripts and superscripts
        cursor = pts_ss_checker(latex, cursor);

        // Skip spaces
        cursor = ignore_spaces_left(latex, cursor);

        std::smatch match;
        std::string remaining_text = latex.substr(0, cursor);

        if (std::regex_search(remaining_text, match, right_command_pattern)) {
            cursor -= 1;
            cursor = move_cursor_past_left_right(latex, cursor, -1);
        } else if (std::regex_search(remaining_text, match, number_pattern)) {
            cursor -= static_cast<int>(match[0].length());
        } else if (std::regex_search(remaining_text, match, latex_pattern)) {
            cursor -= static_cast<int>(match[0].length());
        } else {
            cursor -= 1;
        }

        cursor = ignore_spaces_left(latex, cursor);

        return cursor;
    }
}
